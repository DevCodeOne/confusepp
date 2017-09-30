#pragma once

#include <cstring>

#include <map>
#include <memory>
#include <sstream>
#include <utility>
#include <variant>
#include <vector>

#include <confuse.h>

template<typename T>
class confuse_value;
class confuse_section;

template<typename T>
class confuse_list final : public std::vector<T> {
    public:
        confuse_list(const std::initializer_list<T> &values = {});
        confuse_list(const confuse_list &list);
        confuse_list(confuse_list &&list);
        virtual ~confuse_list() = default;

        confuse_list &operator=(confuse_list list);

        void swap(confuse_list &other);
        char *default_value();
        const char *default_value() const;
    private:
        template<typename F>
        void update_list(cfg_t *parent, const std::string &identifier, F f);

        size_t m_buffer_size = 0;
        std::unique_ptr<char[]> m_default_value_buf = nullptr;

        template<typename E>
        friend class confuse_value;
};

class confuse_element {
    public:
        confuse_element(const std::string &identifier);
        confuse_element(const confuse_element &element);
        confuse_element(confuse_element &&element);

        const std::string &identifier() const;
    private:
        void parent(const confuse_section *parent);
        const confuse_section *parent() const;

        const confuse_section *m_parent = nullptr;
        const std::string m_identifier;

        template<typename T>
        friend class confuse_value;
        friend class confuse_section;
};

template<typename T>
class confuse_value final : public confuse_element {
    public:
        confuse_value(const std::string &identifier);
        confuse_value(const std::string &identifier, const T &default_value);
        confuse_value(const confuse_value &element);
        confuse_value(confuse_value &&element);
        ~confuse_value() = default;

        confuse_value &operator=(const confuse_value &element) = delete;
        confuse_value &operator=(confuse_value &&element) = delete;

        const T &value() const;
    private:
        cfg_opt_t get_confuse_representation() const;

        mutable T m_value;
        const bool m_has_default_value;

        friend class confuse_section;
};

class confuse_section : public confuse_element {
    public:
        using variant_type = std::variant<confuse_section, confuse_value<int>,
              confuse_value<float>, confuse_value<bool>, confuse_value<std::string>,
              confuse_value<confuse_list<int>>, confuse_value<confuse_list<float>>,
              confuse_value<confuse_list<bool>>, confuse_value<confuse_list<std::string>>>;
        using option_storage = std::vector<std::unique_ptr<cfg_opt_t []>>;

        confuse_section(const std::string &identifier, const std::initializer_list<variant_type> &values);
        confuse_section(const confuse_section &section);
        confuse_section(confuse_section &&section);
        virtual ~confuse_section() = default;

        const variant_type &operator[](const std::string &identifier) const;
        confuse_section &operator=(const confuse_section &section) = delete;
        confuse_section &operator=(confuse_section &&section) = delete;

        template<typename T>
        const T &get(const std::string &identifier) const;
    protected:
        cfg_opt_t get_confuse_representation(option_storage &opt_storage) const;
        virtual cfg_t *section_handle() const;
    private:
        std::map<std::string, variant_type> m_values;

        template<typename T>
        friend class confuse_value;
};

class confuse_root final : public confuse_section {
    public:
        confuse_root(const std::initializer_list<variant_type> &values);
        confuse_root(const confuse_root &root);
        confuse_root(confuse_root &&root);
        virtual ~confuse_root() = default;

        confuse_root &operator=(const confuse_root &root) = delete;
        confuse_root &operator=(confuse_root &&root) = delete;
        explicit operator bool() const;

        void config_handle(cfg_t *config_handle);
    protected:
        virtual cfg_t *section_handle() const;
    private:
        cfg_t *m_section_handle = nullptr;

        friend class confuse_config;
};

template<typename T>
void swap(confuse_list<T> &lhs, confuse_list<T> &rhs) {
    lhs.swap(rhs);
}

template<typename T>
confuse_list<T>::confuse_list(const std::initializer_list<T> &values) : std::vector<T>(values) {
    std::stringstream stream("{");

    stream << "{";
    auto it = values.begin();
    while (it != values.end()) {
        if constexpr (std::is_same_v<T, std::decay_t<std::string>>) {
            stream << '\"' << *it << '\"';
        } else {
            stream << *it;
        }

        ++it;
        if (it != values.end()) {
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
confuse_list<T>::confuse_list(const confuse_list &list) : std::vector<T>(list),
    m_buffer_size(list.m_buffer_size),
    m_default_value_buf(std::make_unique<char[]>(m_buffer_size)) {

    if (m_buffer_size && m_default_value_buf && list.m_default_value_buf) {
        std::strncpy(m_default_value_buf.get(), list.m_default_value_buf.get(), m_buffer_size);
    }
}

template<typename T>
confuse_list<T>::confuse_list(confuse_list &&list) : std::vector<T>(list),
    m_buffer_size(list.m_buffer_size),
    m_default_value_buf(std::move(list.m_default_value_buf)) {
    list.m_default_value_buf = nullptr;
}

template<typename T>
confuse_list<T> &confuse_list<T>::operator=(confuse_list list) {
    swap(list);
    return *this;
}

template<typename T>
const char *confuse_list<T>::default_value() const {
    return m_default_value_buf.get();
}

template<typename T>
char *confuse_list<T>::default_value() {
    return m_default_value_buf.get();
}

template<typename T>
void confuse_list<T>::swap(confuse_list &other) {
    using std::swap;

    swap((std::vector<T> &) *this, (std::vector<T> &) other);
    swap(m_buffer_size, other.m_buffer_size);
    swap(m_default_value_buf, other.m_default_value_buf);
}

template<typename T>
template<typename F>
void confuse_list<T>::update_list(cfg_t *parent, const std::string &identifier, F f) {
    std::vector<T>::clear();
    size_t number_of_elements = cfg_size(parent, identifier.c_str());
    std::vector<T>::reserve(number_of_elements);
    for (unsigned int i = 0; i < number_of_elements; ++i) {
        std::vector<T>::push_back(f(parent, identifier.c_str(), i));
    }
}

template<typename T>
confuse_value<T>::confuse_value(const std::string &identifier)
    : confuse_element(identifier), m_has_default_value(false) {
}

template<typename T>
confuse_value<T>::confuse_value(const std::string &identifier, const T &default_value)
    : confuse_element(identifier), m_value(default_value), m_has_default_value(true) {
}

template<typename T>
confuse_value<T>::confuse_value(const confuse_value<T> &element)
    : confuse_element(element), m_value(element.m_value), m_has_default_value(element.m_has_default_value) {
}

template<typename T>
confuse_value<T>::confuse_value(confuse_value<T> &&element)
    : confuse_element(element), m_value(std::move(element.m_value)),
    m_has_default_value(std::move(element.m_has_default_value)) {
}

template<>
inline cfg_opt_t confuse_value<int>::get_confuse_representation() const {
    cfg_opt_t tmp = CFG_INT(m_identifier.c_str(), m_value, m_has_default_value ? CFGF_NODEFAULT : CFGF_NONE);
    return tmp;
}

template<>
inline cfg_opt_t confuse_value<float>::get_confuse_representation() const {
    cfg_opt_t tmp = CFG_FLOAT(m_identifier.c_str(), m_value, m_has_default_value ? CFGF_NODEFAULT : CFGF_NONE);
    return tmp;
}

template<>
inline cfg_opt_t confuse_value<bool>::get_confuse_representation() const {
    cfg_opt_t tmp = CFG_BOOL(m_identifier.c_str(), (cfg_bool_t) m_value, m_has_default_value ? CFGF_NODEFAULT : CFGF_NONE);
    return tmp;
}

template<>
inline cfg_opt_t confuse_value<std::string>::get_confuse_representation() const {
    cfg_opt_t tmp = CFG_STR(m_identifier.c_str(), m_value.c_str(), m_has_default_value ? CFGF_NODEFAULT : CFGF_NONE);
    return tmp;
}

template<>
inline cfg_opt_t confuse_value<confuse_list<int>>::get_confuse_representation() const {
    cfg_opt_t tmp = CFG_INT_LIST(m_identifier.c_str(), m_value.default_value(), (m_has_default_value ? CFGF_NODEFAULT : CFGF_NONE) | CFGF_LIST);
    return tmp;
}

template<>
inline cfg_opt_t confuse_value<confuse_list<float>>::get_confuse_representation() const {
    cfg_opt_t tmp = CFG_FLOAT_LIST(m_identifier.c_str(), m_value.default_value(), (m_has_default_value ? CFGF_NODEFAULT : CFGF_NONE) | CFGF_LIST);
    return tmp;
}

template<>
inline cfg_opt_t confuse_value<confuse_list<bool>>::get_confuse_representation() const {
    cfg_opt_t tmp = CFG_BOOL_LIST(m_identifier.c_str(), m_value.default_value(), (m_has_default_value ? CFGF_NODEFAULT : CFGF_NONE) | CFGF_LIST);
    return tmp;
}

template<>
inline cfg_opt_t confuse_value<confuse_list<std::string>>::get_confuse_representation() const {
    cfg_opt_t tmp = CFG_STR_LIST(m_identifier.c_str(), m_value.default_value(), (m_has_default_value ? CFGF_NODEFAULT : CFGF_NONE) | CFGF_LIST);
    return tmp;
}

template<>
inline const int &confuse_value<int>::value() const {
    if (m_parent && m_parent->section_handle()) {
        m_value = cfg_getint(m_parent->section_handle(), m_identifier.c_str());
    }
    return m_value;
}

template<>
inline const float &confuse_value<float>::value() const {
    if (m_parent && m_parent->section_handle()) {
        m_value = cfg_getfloat(m_parent->section_handle(), m_identifier.c_str());
    }
    return m_value;
}

template<>
inline const bool &confuse_value<bool>::value() const {
    if (m_parent && m_parent->section_handle()) {
        m_value = cfg_getbool(m_parent->section_handle(), m_identifier.c_str());
    }
    return m_value;
}

template<>
inline const std::string &confuse_value<std::string>::value() const {
    if (m_parent && m_parent->section_handle()) {
        m_value = cfg_getstr(m_parent->section_handle(), m_identifier.c_str());
    }
    return m_value;
}

template<>
inline const confuse_list<int> &confuse_value<confuse_list<int>>::value() const {
    m_value.update_list(m_parent->section_handle(), m_identifier, cfg_getnint);
    return m_value;
}

template<>
inline const confuse_list<float> &confuse_value<confuse_list<float>>::value() const {
    m_value.update_list(m_parent->section_handle(), m_identifier, cfg_getnfloat);
    return m_value;
}

template<>
inline const confuse_list<bool> &confuse_value<confuse_list<bool>>::value() const {
    m_value.update_list(m_parent->section_handle(), m_identifier, cfg_getnbool);
    return m_value;
}

template<>
inline const confuse_list<std::string> &confuse_value<confuse_list<std::string>>::value() const {
    m_value.update_list(m_parent->section_handle(), m_identifier, cfg_getnstr);
    return m_value;
}

template<typename T>
const T &confuse_section::get(const std::string &identifier) const {
    return std::get<T>(operator[](identifier));
}
