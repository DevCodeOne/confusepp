#pragma once

#include <cstring>

#include <experimental/filesystem>
#include <map>
#include <memory>
#include <sstream>
#include <utility>
#include <variant>
#include <vector>

#include <confuse.h>

namespace confusepp {

    template<typename T>
    class Option;
    class Section;
    class Multisection;

    template<typename T>
    class List final : public std::vector<T> {
       public:
        template<typename... Args>
        List(Args... args);
        List(const List& list);
        List(List&& list);
        virtual ~List() = default;

        List& operator=(List list);

        void swap(List& other);
        char* default_value();
        const char* default_value() const;

       private:
        template<typename F>
        void update_list(cfg_t* parent, const std::string& identifier, F f);

        size_t m_buffer_size = 0;
        std::unique_ptr<char[]> m_default_value_buf = nullptr;

        template<typename E>
        friend class Option;
    };

    class Element {
       public:
        Element(const std::string& identifier);
        virtual ~Element() = default;

        const std::string& identifier() const;

       private:
        std::string m_identifier;
    };

    template<typename T>
    class Option final : public Element {
       public:
        Option(const std::string& identifier);
        virtual ~Option() = default;

        template<typename... Args>
        const Option<T>& default_value(Args... args);
        const T& value() const;

       private:
        const T& value(cfg_t* parent) const;
        cfg_opt_t get_confuse_representation() const;
        void load(cfg_t* parent_handle);

        mutable T m_value;
        bool m_has_default_value;

        friend class Section;
        friend class Multisection;
        friend class ConfigFormat;
    };

    class Section : public Element {
       public:
        using variant_type =
            std::variant<Section, Multisection, Option<int>, Option<float>, Option<bool>, Option<std::string>,
                         Option<List<int>>, Option<List<float>>, Option<List<bool>>, Option<List<std::string>>>;
        using option_storage = std::vector<std::unique_ptr<cfg_opt_t[]>>;

        Section(const std::string& identifier);
        virtual ~Section() = default;

        template<typename T>
        std::optional<T> get(const std::string& identifier) const;
        std::optional<variant_type> operator[](const std::string& identifier) const;
        const std::string& title() const;
        template<typename... Args>
        Section& values(Args... args);

       protected:
        Section& title(const std::string& title);
        cfg_opt_t get_confuse_representation(option_storage& opt_storage) const;
        virtual void load(cfg_t* parent_handle);

       private:
        std::map<std::string, variant_type> m_values;
        std::string m_title;

        void add_children(std::vector<variant_type> values);

        template<typename T>
        friend class Option;
        friend class Multisection;
        friend class ConfigFormat;
    };

    class Multisection final : public Element {
       public:
        using variant_type = Section::variant_type;
        using option_storage = Section::option_storage;

        Multisection(const std::string& identifier);
        virtual ~Multisection() = default;

        std::optional<Section> operator[](const std::string& title) const;
        template<typename... Args>
        Multisection& values(Args... args);

       private:
        cfg_opt_t get_confuse_representation(option_storage& opt_storage) const;
        void load(cfg_t* parent_handle);

        std::vector<variant_type> m_values;
        mutable std::map<std::string, Section> m_sections;

        template<typename T>
        friend class Option;
        friend class Section;
        friend class ConfigFormat;
    };

    class ConfigFormat final : public Section {
       public:
        ConfigFormat(const std::initializer_list<variant_type>& values);
        virtual ~ConfigFormat() = default;

        void load(cfg_t* parent_handle) override;

       private:
        friend class Config;
    };

    template<typename T>
    void swap(List<T>& lhs, List<T>& rhs) {
        lhs.swap(rhs);
    }

    template<typename T>
    template<typename... Args>
    List<T>::List(Args... args) : std::vector<T>(std::initializer_list<T>{args...}) {
        std::stringstream stream("{");

        stream << "{";
        auto it = std::vector<T>::cbegin();
        while (it != std::vector<T>::cend()) {
            if constexpr (std::is_same_v<std::decay_t<T>, std::string>) {
                stream << '\"' << *it << '\"';
            } else {
                if constexpr (std::is_same_v<std::decay_t<T>, bool>) {
                    stream << (*it ? "true" : "false") << std::endl;
                } else {
                    stream << *it;
                }
            }

            ++it;
            if (it != std::vector<T>::cend()) {
                stream << ", ";
            }
        }
        stream << "}";

        m_buffer_size = stream.str().size() + 1;
        m_default_value_buf = std::make_unique<char[]>(m_buffer_size);

        if (m_buffer_size && m_default_value_buf) {
            std::strncpy(m_default_value_buf.get(), stream.str().c_str(), m_buffer_size);
        }
    }

    template<typename T>
    List<T>::List(const List& list)
        : std::vector<T>(list),
          m_buffer_size(list.m_buffer_size),
          m_default_value_buf(std::make_unique<char[]>(m_buffer_size)) {
        if (m_buffer_size && m_default_value_buf && list.m_default_value_buf) {
            std::strncpy(m_default_value_buf.get(), list.m_default_value_buf.get(), m_buffer_size);
        }
    }

    template<typename T>
    List<T>::List(List&& list)
        : std::vector<T>(std::move(list)),
          m_buffer_size(std::move(list.m_buffer_size)),
          m_default_value_buf(std::move(list.m_default_value_buf)) {
        list.m_default_value_buf = nullptr;
    }

    template<typename T>
    List<T>& List<T>::operator=(List list) {
        swap(list);
        return *this;
    }

    template<typename T>
    const char* List<T>::default_value() const {
        return m_default_value_buf.get();
    }

    template<typename T>
    char* List<T>::default_value() {
        return m_default_value_buf.get();
    }

    template<typename T>
    void List<T>::swap(List& other) {
        using std::swap;

        swap((std::vector<T>&)*this, (std::vector<T>&)other);
        swap(m_buffer_size, other.m_buffer_size);
        swap(m_default_value_buf, other.m_default_value_buf);
    }

    template<typename T>
    template<typename F>
    void List<T>::update_list(cfg_t* parent, const std::string& identifier, F f) {
        std::vector<T>::clear();
        size_t number_of_elements = cfg_size(parent, identifier.c_str());
        std::vector<T>::reserve(number_of_elements);
        for (unsigned int i = 0; i < number_of_elements; ++i) {
            std::vector<T>::push_back(f(parent, identifier.c_str(), i));
        }
    }

    template<typename T>
    Option<T>::Option(const std::string& identifier) : Element(identifier), m_has_default_value(false) {}

    template<>
    inline cfg_opt_t Option<int>::get_confuse_representation() const {
        cfg_opt_t tmp = CFG_INT(identifier().c_str(), m_value, m_has_default_value ? CFGF_NONE : CFGF_NODEFAULT);
        return tmp;
    }

    template<>
    inline cfg_opt_t Option<float>::get_confuse_representation() const {
        cfg_opt_t tmp = CFG_FLOAT(identifier().c_str(), m_value, m_has_default_value ? CFGF_NONE : CFGF_NODEFAULT);
        return tmp;
    }

    template<>
    inline cfg_opt_t Option<bool>::get_confuse_representation() const {
        cfg_opt_t tmp =
            CFG_BOOL(identifier().c_str(), (cfg_bool_t)m_value, m_has_default_value ? CFGF_NONE : CFGF_NODEFAULT);
        return tmp;
    }

    template<>
    inline cfg_opt_t Option<std::string>::get_confuse_representation() const {
        cfg_opt_t tmp =
            CFG_STR(identifier().c_str(), m_value.c_str(), m_has_default_value ? CFGF_NONE : CFGF_NODEFAULT);
        return tmp;
    }

    template<>
    inline cfg_opt_t Option<List<int>>::get_confuse_representation() const {
        cfg_opt_t tmp = CFG_INT_LIST(identifier().c_str(), m_value.default_value(),
                                     (m_has_default_value ? CFGF_NONE : CFGF_NODEFAULT) | CFGF_LIST);
        return tmp;
    }

    template<>
    inline cfg_opt_t Option<List<float>>::get_confuse_representation() const {
        cfg_opt_t tmp = CFG_FLOAT_LIST(identifier().c_str(), m_value.default_value(),
                                       (m_has_default_value ? CFGF_NONE : CFGF_NODEFAULT) | CFGF_LIST);
        return tmp;
    }

    template<>
    inline cfg_opt_t Option<List<bool>>::get_confuse_representation() const {
        cfg_opt_t tmp = CFG_BOOL_LIST(identifier().c_str(), m_value.default_value(),
                                      (m_has_default_value ? CFGF_NONE : CFGF_NODEFAULT) | CFGF_LIST);
        return tmp;
    }

    template<>
    inline cfg_opt_t Option<List<std::string>>::get_confuse_representation() const {
        cfg_opt_t tmp = CFG_STR_LIST(identifier().c_str(), m_value.default_value(),
                                     (m_has_default_value ? CFGF_NONE : CFGF_NODEFAULT) | CFGF_LIST);
        return tmp;
    }

    template<typename T>
    template<typename... Args>
    const Option<T>& Option<T>::default_value(Args... args) {
        m_has_default_value = true;
        m_value = T(args...);

        return *this;
    }

    template<typename T>
    void Option<T>::load(cfg_t* parent_handle) {
        value(parent_handle);
    }

    template<typename T>
    const T& Option<T>::value() const {
        return m_value;
    }

    template<>
    inline const int& Option<int>::value(cfg_t* parent_handle) const {
        if (parent_handle) {
            m_value = cfg_getint(parent_handle, identifier().c_str());
        }
        return m_value;
    }

    template<>
    inline const float& Option<float>::value(cfg_t* parent_handle) const {
        if (parent_handle) {
            m_value = cfg_getfloat(parent_handle, identifier().c_str());
        }
        return m_value;
    }

    template<>
    inline const bool& Option<bool>::value(cfg_t* parent_handle) const {
        if (parent_handle) {
            m_value = cfg_getbool(parent_handle, identifier().c_str());
        }
        return m_value;
    }

    template<>
    inline const std::string& Option<std::string>::value(cfg_t* parent_handle) const {
        if (parent_handle) {
            m_value = cfg_getstr(parent_handle, identifier().c_str());
        }
        return m_value;
    }

    template<>
    inline const List<int>& Option<List<int>>::value(cfg_t* parent_handle) const {
        if (parent_handle) {
            m_value.update_list(parent_handle, identifier(), cfg_getnint);
        }
        return m_value;
    }

    template<>
    inline const List<float>& Option<List<float>>::value(cfg_t* parent_handle) const {
        if (parent_handle) {
            m_value.update_list(parent_handle, identifier(), cfg_getnfloat);
        }
        return m_value;
    }

    template<>
    inline const List<bool>& Option<List<bool>>::value(cfg_t* parent_handle) const {
        if (parent_handle) {
            m_value.update_list(parent_handle, identifier(), cfg_getnbool);
        }
        return m_value;
    }

    template<>
    inline const List<std::string>& Option<List<std::string>>::value(cfg_t* parent_handle) const {
        if (parent_handle) {
            m_value.update_list(parent_handle, identifier(), cfg_getnstr);
        }
        return m_value;
    }

    template<typename T>
    std::optional<T> Section::get(const std::string& identifier) const {
        if (auto value = m_values.find(identifier); value != m_values.cend()) {
            return std::optional<T>{std::get<T>(value->second)};
        }
        return {};
    }

    template<typename... Args>
    Section& Section::values(Args... args) {
        m_values.clear();
        add_children({args...});

        return *this;
    }

    template<typename... Args>
    Multisection& Multisection::values(Args... args) {
        m_values = std::vector<variant_type>({args...});
        return *this;
    }

}  // namespace confusepp
