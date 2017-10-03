#pragma once

#include <cstring>

#include <map>
#include <memory>
#include <sstream>
#include <utility>
#include <variant>
#include <vector>

#include <confuse.h>

namespace confuse {

    template<typename T>
    class Option;

    class Section;

    class Multisection;

    template<typename T>
    class List final : public std::vector<T> {
    public:
        template<typename ... Args>
        List(Args
        ... args);

        List(const List& list);

        List(List
        &&list);

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
        friend
        class Option;
    };

    class Parent {
    public:
        virtual cfg_t* section_handle() const = 0;

        virtual ~Parent() = default;
    };

    class Element {
    public:
        Element(const std::string& identifier);

        Element(const Element& element);

        Element(Element
        &&element);

        const std::string& identifier() const;

    private:
        void parent(const Parent* parent);

        const Parent* parent() const;

        const Parent* m_parent = nullptr;
        const std::string m_identifier;

        template<typename T>
        friend
        class Option;

        friend class Section;

        friend class Multisection;
    };

    template<typename T>
    class Option final : public Element {
    public:
        Option(const std::string& identifier);

        Option(const Option& element);

        Option(Option
        &&element);

        ~Option() = default;

        Option& operator=(const Option& element) = delete;

        Option& operator=(Option&& element) = delete;

        template<typename ... Args>
        const Option<T>& default_value(Args ... args);

        const T& value() const;

    private:
        cfg_opt_t get_confuse_representation() const;

        mutable T m_value;
        bool m_has_default_value;

        friend class Section;

        friend class Multisection;
    };

    class Section : public Element, public Parent {
    public:
        using variant_type = std::variant<Section, Multisection, Option<int>,
                Option<float>, Option<bool>, Option<std::string>,
                Option<List<int>>, Option<List<float>>,
                Option<List<bool>>, Option<List<std::string>>>;
        using option_storage = std::vector<std::unique_ptr<cfg_opt_t[]>>;

        Section(const std::string& identifier, const std::initializer_list<variant_type>& values);

        Section(const Section& section);

        Section(Section
        &&section);

        virtual ~Section() = default;

        Section& operator=(const Section& section) = delete;

        Section& operator=(Section&& section) = delete;

        template<typename T>
        const T& get(const std::string& identifier) const;

        Section& title(const std::string& title);

    protected:
        cfg_opt_t get_confuse_representation(option_storage& opt_storage) const;

        virtual cfg_t* section_handle() const override;

    private:
        std::map<std::string, variant_type> m_values;
        std::string m_title;

        void add_children(std::vector<variant_type> values);

        template<typename T>
        friend
        class Option;

        friend class Multisection;
    };

    class Root final : public Section {
    public:
        Root(const std::initializer_list<variant_type>& values);

        Root(const Root& root);

        Root(Root
        &&root);

        virtual ~Root() = default;

        Root& operator=(const Root& root) = delete;

        Root& operator=(Root&& root) = delete;

        explicit operator bool() const;

        void config_handle(cfg_t* config_handle);

    private:
        cfg_t* m_section_handle = nullptr;

        virtual cfg_t* section_handle() const override;

        friend class Config;
    };

    class Multisection final : public Element, public Parent {
    public:
        using variant_type = Section::variant_type;
        using option_storage = Section::option_storage;

        Multisection(const std::string& identifier, const std::initializer_list<variant_type>& values);

        Multisection(const Multisection& section);

        Multisection(Multisection
        &&section);

        virtual ~Multisection() = default;

        std::optional<Section> operator[](const std::string& title) const;

        Multisection& operator=(const Section& section) = delete;

        Multisection& operator=(Section&& section) = delete;

    private:
        std::vector<variant_type> m_values;
        mutable std::map<std::string, Section> m_sections;

        cfg_opt_t get_confuse_representation(option_storage& opt_storage) const;

        virtual cfg_t* section_handle() const override;

        template<typename T>
        friend
        class Option;

        friend class Section;
    };

    template<typename T>
    void swap(List<T>& lhs, List<T>& rhs) {
        lhs.swap(rhs);
    }

    template<typename T>
    template<typename ... Args>
    List<T>::List(Args
    ... args) :
    std::vector<T>(std::initializer_list<T>{args ...}) {
            std::stringstream stream("{");

            stream << "{";
            auto it = std::vector<T>::cbegin();
            while (it != std::vector<T>::cend()) {
                if constexpr (std::is_same_v<T, std::decay_t<std::string>>)
                {
                    stream << '\"' << *it << '\"';
                } else {
                    stream << *it;
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
    List<T>::List(const List& list) : std::vector<T>(list),
                                                                 m_buffer_size(list.m_buffer_size),
                                                                 m_default_value_buf(
                                                                         std::make_unique<char[]>(m_buffer_size)) {

        if (m_buffer_size && m_default_value_buf && list.m_default_value_buf) {
            std::strncpy(m_default_value_buf.get(), list.m_default_value_buf.get(), m_buffer_size);
        }
    }

    template<typename T>
    List<T>::List(List
    &&list) :

    std::vector<T> (list),
    m_buffer_size(list.m_buffer_size),
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

        swap((std::vector<T>&) *this, (std::vector<T>&) other);
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
    Option<T>::Option(const std::string& identifier)
            : Element(identifier), m_has_default_value(false) {
    }

    template<typename T>
    Option<T>::Option(const Option<T>& element)
            : Element(element), m_value(element.m_value), m_has_default_value(element.m_has_default_value) {
    }

    template<typename T>
    Option<T>::Option(Option<T>
    &&element)
    :

    Element (element), m_value(std::move(element.m_value)),
    m_has_default_value(std::move(element.m_has_default_value)) {
    }

    template<>
    inline cfg_opt_t Option<int>::get_confuse_representation() const {
        cfg_opt_t tmp = CFG_INT(m_identifier.c_str(), m_value, m_has_default_value ? CFGF_NONE : CFGF_NODEFAULT);
        return tmp;
    }

    template<>
    inline cfg_opt_t Option<float>::get_confuse_representation() const {
        cfg_opt_t tmp = CFG_FLOAT(m_identifier.c_str(), m_value, m_has_default_value ? CFGF_NONE : CFGF_NODEFAULT);
        return tmp;
    }

    template<>
    inline cfg_opt_t Option<bool>::get_confuse_representation() const {
        cfg_opt_t tmp = CFG_BOOL(m_identifier.c_str(), (cfg_bool_t) m_value,
                                 m_has_default_value ? CFGF_NONE : CFGF_NODEFAULT);
        return tmp;
    }

    template<>
    inline cfg_opt_t Option<std::string>::get_confuse_representation() const {
        cfg_opt_t tmp = CFG_STR(m_identifier.c_str(), m_value.c_str(),
                                m_has_default_value ? CFGF_NONE : CFGF_NODEFAULT);
        return tmp;
    }

    template<>
    inline cfg_opt_t Option<List<int>>::get_confuse_representation() const {
        cfg_opt_t tmp = CFG_INT_LIST(m_identifier.c_str(), m_value.default_value(),
                                     (m_has_default_value ? CFGF_NONE : CFGF_NODEFAULT) | CFGF_LIST);
        return tmp;
    }

    template<>
    inline cfg_opt_t Option<List<float>>::get_confuse_representation() const {
        cfg_opt_t tmp = CFG_FLOAT_LIST(m_identifier.c_str(), m_value.default_value(),
                                       (m_has_default_value ? CFGF_NONE : CFGF_NODEFAULT) | CFGF_LIST);
        return tmp;
    }

    template<>
    inline cfg_opt_t Option<List<bool>>::get_confuse_representation() const {
        cfg_opt_t tmp = CFG_BOOL_LIST(m_identifier.c_str(), m_value.default_value(),
                                      (m_has_default_value ? CFGF_NONE : CFGF_NODEFAULT) | CFGF_LIST);
        return tmp;
    }

    template<>
    inline cfg_opt_t Option<List<std::string>>::get_confuse_representation() const {
        cfg_opt_t tmp = CFG_STR_LIST(m_identifier.c_str(), m_value.default_value(),
                                     (m_has_default_value ? CFGF_NONE : CFGF_NODEFAULT) | CFGF_LIST);
        return tmp;
    }

    template<typename T>
    template<typename ... Args>
    const Option<T>& Option<T>::default_value(Args ... args) {
        m_has_default_value = true;
        m_value = T(args ...);

        return *this;
    }

    template<>
    inline const int& Option<int>::value() const {
        if (m_parent && m_parent->section_handle()) {
            m_value = cfg_getint(m_parent->section_handle(), m_identifier.c_str());
        }
        return m_value;
    }

    template<>
    inline const float& Option<float>::value() const {
        if (m_parent && m_parent->section_handle()) {
            m_value = cfg_getfloat(m_parent->section_handle(), m_identifier.c_str());
        }
        return m_value;
    }

    template<>
    inline const bool& Option<bool>::value() const {
        if (m_parent && m_parent->section_handle()) {
            m_value = cfg_getbool(m_parent->section_handle(), m_identifier.c_str());
        }
        return m_value;
    }

    template<>
    inline const std::string& Option<std::string>::value() const {
        if (m_parent && m_parent->section_handle()) {
            m_value = cfg_getstr(m_parent->section_handle(), m_identifier.c_str());
        }
        return m_value;
    }

    template<>
    inline const List<int>& Option<List<int>>::value() const {
        m_value.update_list(m_parent->section_handle(), m_identifier, cfg_getnint);
        return m_value;
    }

    template<>
    inline const List<float>& Option<List<float>>::value() const {
        m_value.update_list(m_parent->section_handle(), m_identifier, cfg_getnfloat);
        return m_value;
    }

    template<>
    inline const List<bool>& Option<List<bool>>::value() const {
        m_value.update_list(m_parent->section_handle(), m_identifier, cfg_getnbool);
        return m_value;
    }

    template<>
    inline const List<std::string>& Option<List<std::string>>::value() const {
        m_value.update_list(m_parent->section_handle(), m_identifier, cfg_getnstr);
        return m_value;
    }

    template<typename T>
    const T& Section::get(const std::string& identifier) const {
        return std::get<T>(m_values.at(identifier));
    }
}