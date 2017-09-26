#pragma once

#include <cstring>

#include <istream>
#include <ostream>
#include <map>
#include <memory>
#include <sstream>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

#include <confuse.h>

// TODO implement flags
class confuse_section_flags {

};

// TODO implement flags
class confuse_element_flags {

};

// TODO implement this
template<typename T>
class confuse_list final : public std::vector<T> {
    public:
        confuse_list(const std::initializer_list<T> &values = {});
        confuse_list(const confuse_list &list);
        confuse_list(confuse_list &&list);
        const char *default_value() const;
        char *default_value();

        confuse_list &operator=(confuse_list list);

        void swap(confuse_list &other);
        virtual ~confuse_list() = default;
    private:
        size_t m_buffer_size = 0;
        std::unique_ptr<char[]> m_default_value_buf = nullptr;
};

class confuse_section;

// Add some sort of mechanism for the default value of lists (string)
class confuse_element final {
    public:
        using value_type = std::variant<int, float, bool, std::string,
              confuse_list<int>, confuse_list<float>, confuse_list<bool>, confuse_list<std::string>>;

        template<typename T>
        confuse_element(const std::string &identifier, const T &default_value = {},
                bool optional = false);
        confuse_element(const confuse_element &element);
        confuse_element(confuse_element &&element);

        confuse_element &operator=(const confuse_element &element) = delete;
        confuse_element &operator=(confuse_element &&element) = delete;

        const std::string &identifier() const;
        template<typename T>
        const T &value() const;
    private:
        cfg_opt_t get_confuse_representation() const;
        void parent(const confuse_section *parent);
        const confuse_section *parent() const;

        mutable value_type m_value;
        const confuse_section *m_parent = nullptr;
        const bool m_optional;
        const std::string m_identifier;

        friend class confuse_section;
};

class confuse_section {
    public:
        using variant_type = std::variant<confuse_section, confuse_element>;
        using option_storage = std::vector<std::unique_ptr<cfg_opt_t []>>;

        confuse_section(const std::string &identifier,
                const std::initializer_list<variant_type> &values, bool optional = false);
        confuse_section(const confuse_section &section);
        confuse_section(confuse_section &&section);
        virtual ~confuse_section() = default;

        const std::string &identifier() const;
        const variant_type &operator[](const std::string &identifier) const;
        template<typename T>
        const T &get(const std::string &identifier) const;
        confuse_section &operator=(const confuse_section &section) = delete;
        confuse_section &operator=(confuse_section &&section) = delete;
    protected:
        cfg_opt_t get_confuse_representation(option_storage &opt_storage) const;
        void parent(const confuse_section *parent);
        const confuse_section *parent() const;
        virtual cfg_t *section_handle() const;
    private:

        std::map<std::string, variant_type> m_values;
        const confuse_section *m_parent = nullptr;

        const bool m_optional;
        const std::string m_identifier;

        friend class confuse_element;
};

// TODO complete implementation (operators and constructors)
class confuse_root final : public confuse_section {
    public:
        confuse_root(const std::initializer_list<variant_type> &values);
        confuse_root(const confuse_root &root);
        confuse_root(confuse_root &&root);

        virtual ~confuse_root() = default;

        void config_handle(cfg_t *config_handle);
        virtual cfg_t *section_handle() const;

        confuse_root &operator=(const confuse_root &root) = delete;
        confuse_root &operator=(confuse_root &&root) = delete;
        explicit operator bool() const;
    private:
        cfg_t *m_section_handle = nullptr;

        friend class confuse_config;
};

template<typename T>
void swap(confuse_list<T> &lhs, confuse_list<T> &rhs) {
    lhs.swap(rhs);
}

// TODO create specialization for std::string
template<typename T>
confuse_list<T>::confuse_list(const std::initializer_list<T> &values)
    : std::vector<T>(values) {
    std::stringstream stream;

    stream << "{";
    auto it = values.begin();
    while (it != values.end()) {
        stream << *it;

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
        std::strncpy(m_default_value_buf.get(),
                list.m_default_value_buf.get(), m_buffer_size);
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

// TODO correct datatypes and add missing ones
// remove duplicated code somehow
template<typename T>
const T &confuse_element::value() const {
    auto &stored_value = m_value;
    cfg_t *parent = m_parent->section_handle();

    if (parent) {
        if (std::holds_alternative<int>(stored_value)) {
            stored_value = value_type((int) cfg_getint(parent, m_identifier.c_str()));
        } else if (std::holds_alternative<float>(stored_value)) {
            stored_value = value_type((float) cfg_getfloat(parent, m_identifier.c_str()));
        } else if (std::holds_alternative<bool>(stored_value)) {
            stored_value = cfg_getbool(parent, m_identifier.c_str());
        } else if (std::holds_alternative<std::string>(stored_value)) {
            stored_value = std::string(cfg_getstr(parent, m_identifier.c_str()));
        } else if (std::holds_alternative<confuse_list<int>>(stored_value)) {
            confuse_list<int> list;
            size_t number_of_elements = cfg_size(parent, m_identifier.c_str());
            list.reserve(number_of_elements);

            for (unsigned int i = 0; i < number_of_elements; i++) {
                list.push_back(cfg_getnint(parent, m_identifier.c_str(), i));
            }

            stored_value = list;
        } else if (std::holds_alternative<confuse_list<float>>(stored_value)) {
            confuse_list<float> list;
            size_t number_of_elements = cfg_size(parent, m_identifier.c_str());
            list.reserve(number_of_elements);

            for (unsigned int i = 0; i < number_of_elements; i++) {
                list.push_back(cfg_getnfloat(parent, m_identifier.c_str(), i));
            }

            stored_value = list;
        } else if (std::holds_alternative<confuse_list<bool>>(stored_value)) {
            confuse_list<bool> list;
            size_t number_of_elements = cfg_size(parent, m_identifier.c_str());
            list.reserve(number_of_elements);

            for (unsigned int i = 0; i < number_of_elements; i++) {
                list.push_back(cfg_getnbool(parent, m_identifier.c_str(), i));
            }

            stored_value = list;
        } else if (std::holds_alternative<confuse_list<std::string>>(stored_value)) {
            confuse_list<std::string> list;
            size_t number_of_elements = cfg_size(parent, m_identifier.c_str());
            list.reserve(number_of_elements);

            for (unsigned int i = 0; i < number_of_elements; i++) {
                list.push_back(cfg_getnstr(parent, m_identifier.c_str(), i));
            }

            stored_value = list;
        }
    }

    return std::get<T>(stored_value);
}

template<typename T>
confuse_element::confuse_element(const std::string &identifier, const T &default_value, bool optional)
    : m_value(default_value), m_optional(optional), m_identifier(identifier) {
}

template<typename T>
const T &confuse_section::get(const std::string &identifier) const {
    return std::get<T>(operator[](identifier));
}


template<typename T>
confuse_element confuse_value(const std::string &identifier, const T &default_value = {}, bool optional = false) {
    return confuse_element(identifier, default_value, optional);
}
