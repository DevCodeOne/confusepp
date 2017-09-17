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

// TODO complete implementation (operators and constructors)
// also add the other datatypes
// replace std::string with const char * and cast this to std::string_view somehow
class confuse_element final {
    public:
        using value_type = std::variant<int, float, bool, std::string>;

        template<typename T>
            confuse_element(const std::string &identifier,
                    const T &default_value = {}, bool optional = false)
            : m_value(default_value), m_optional(optional), m_identifier(identifier) {
            }

        const std::string &identifier() const;

        template<typename T>
            bool holds_type() const {
                return std::holds_alternative<T>(m_value);
            }

        template<typename T>
            const T &as() const {
                return std::get<T>(m_value);
            }
    private:
        cfg_opt_t get_confuse_representation() const;

        std::variant<int, float, bool, std::string> m_value;
        const bool m_optional;
        const std::string m_identifier;

        friend class confuse_section;
        friend class confuse_root;
};

// TODO complete implementation (operators and constructors)
class confuse_section {
    public:
        using variant_type = std::variant<confuse_section, confuse_element>;

        confuse_section(const std::string &identifier,
                const std::initializer_list<variant_type> &values,
                bool optional = false);
        const std::string &identifier() const;
        const variant_type &operator[](const std::string &identifier) const;
        virtual ~confuse_section() = default;
    protected:
        cfg_opt_t get_confuse_representation(std::vector<std::unique_ptr<cfg_opt_t []>> &opt_storage) const;
    private:

        std::map<std::string, variant_type> m_values;
        const bool m_optional;
        const std::string m_identifier;
};

// TODO complete implementation (operators and constructors)
class confuse_root final : public confuse_section {
    public:
        confuse_root(const std::initializer_list<variant_type> &values);
        virtual ~confuse_root() = default;

        explicit operator bool() const;
    private:

        friend class confuse_config;
};

template<typename T>
confuse_element confuse_value(const std::string &identifier, const T &default_value = {}, bool optional = false) {
    return confuse_element(identifier, default_value, optional);
}
