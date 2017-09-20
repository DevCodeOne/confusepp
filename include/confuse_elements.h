#pragma once

#include <confuse.h>

#include <map>
#include <istream>
#include <ostream>
#include <tuple>
#include <variant>
#include <memory>
#include <vector>

class confuse_section_flags {

};

class confuse_element_flags {

};

class confuse_section;

// TODO complete implementation (operators and constructors)
// also add the other datatypes
// replace std::string with const char * and cast this to std::string_view somehow
class confuse_element final {
    public:
        using value_type = std::variant<int, float, bool, std::string>;

        template<typename T>
            confuse_element(const std::string &identifier,
                    const T &default_value = {}, bool optional = false)
            : m_value(default_value), m_optional(optional),
            m_identifier(identifier) {
            }
        confuse_element(const confuse_element &element);
        confuse_element(confuse_element &&element);

        confuse_element &operator=(const confuse_element &element) = delete;
        confuse_element &operator=(confuse_element &&element) = delete;

        const std::string &identifier() const;
        const value_type &value() const;
    private:
        cfg_opt_t get_confuse_representation() const;
        void parent(const confuse_section *parent);
        const confuse_section *parent() const;

        mutable std::variant<int, float, bool, std::string> m_value;
        const confuse_section *m_parent = nullptr;
        const bool m_optional;
        const std::string m_identifier;

        friend class confuse_section;
        friend class confuse_root;
};

// TODO complete implementation (operators and constructors)
class confuse_section {
    public:
        using variant_type = std::variant<confuse_section, confuse_element>;
        using option_storage = std::vector<std::unique_ptr<cfg_opt_t []>>;

        confuse_section(const std::string &identifier,
                const std::initializer_list<variant_type> &values,
                bool optional = false);
        confuse_section(const confuse_section &section);
        confuse_section(confuse_section &&section);

        const std::string &identifier() const;
        const variant_type &operator[](const std::string &identifier) const;
        confuse_section &operator=(const confuse_section &section) = delete;
        confuse_section &operator=(confuse_section &&section) = delete;
        virtual ~confuse_section() = default;
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

        friend class confuse_root;
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
confuse_element confuse_value(const std::string &identifier, const T &default_value = {}, bool optional = false) {
    return confuse_element(identifier, default_value, optional);
}
