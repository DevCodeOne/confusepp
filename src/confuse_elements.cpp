#include <iostream>

#include <cstdio>

#include <confuse.h>

#include "confuse_elements.h"

confuse_element::confuse_element(const confuse_element &element)
    : m_value(element.m_value), m_optional(element.m_optional),
    m_identifier(element.m_identifier) {
}

confuse_element::confuse_element(confuse_element &&element)
    : m_value(std::move(element.m_value)), m_optional(std::move(element.m_optional)),
    m_identifier(std::move(element.m_identifier)) {
}

const std::string &confuse_element::identifier() const {
    return m_identifier;
}

// TODO handle callbacks and the like also handle last case differently.
// Remember to add flags somehow
cfg_opt_t confuse_element::get_confuse_representation() const {
    cfg_opt_t ret;

    auto stored_value = m_value;

    if (std::holds_alternative<int>(stored_value)) {
        cfg_opt_t tmp = CFG_INT(m_identifier.c_str(), std::get<int>(m_value), CFGF_NONE);
        ret = tmp;
    } else if (std::holds_alternative<float>(stored_value)) {
        cfg_opt_t tmp = CFG_FLOAT(m_identifier.c_str(), std::get<float>(m_value), CFGF_NONE);
        ret = tmp;
    } else if (std::holds_alternative<bool>(stored_value)) {
        cfg_opt_t tmp = CFG_BOOL(m_identifier.c_str(), (cfg_bool_t) std::get<bool>(m_value), CFGF_NONE);
        ret = tmp;
    } else if (std::holds_alternative<std::string>(stored_value)) {
        cfg_opt_t tmp = CFG_STR(m_identifier.c_str(), std::get<std::string>(m_value).c_str(), CFGF_NONE);
        ret = tmp;
    }

    return ret;
}

void confuse_element::parent(const confuse_section *parent) {
    m_parent = parent;
}

const confuse_section *confuse_element::parent() const {
    return m_parent;
}

// Implementation of confuse_section
confuse_section::confuse_section(const std::string &identifier,
        const std::initializer_list<variant_type> &values, bool optional)
    : m_optional(optional), m_identifier(identifier) {
        for (auto &current_value : values) {
            if (std::holds_alternative<confuse_element>(current_value)) {
                confuse_element real_type = std::get<confuse_element>(current_value);
                m_values.emplace(real_type.identifier(), real_type);
                std::get<confuse_element>(m_values.at(real_type.identifier())).parent(this);
            } else if (std::holds_alternative<confuse_section>(current_value)) {
                confuse_section real_type = std::get<confuse_section>(current_value);
                m_values.emplace(real_type.identifier(), real_type);
                std::get<confuse_section>(m_values.at(real_type.identifier())).parent(this);
            }
        }
}

confuse_section::confuse_section(const confuse_section &section)
    : m_values(section.m_values), m_optional(section.m_optional),
    m_identifier(section.m_identifier) {
    for (auto &value : m_values) {
        auto &current_value = value.second;
        if (std::holds_alternative<confuse_element>(current_value)) {
            std::get<confuse_element>(current_value).parent(this);
        } else if (std::holds_alternative<confuse_section>(current_value)) {
            std::get<confuse_section>(current_value).parent(this);
        }
    }
}

confuse_section::confuse_section(confuse_section &&section)
    : m_values(std::move(section.m_values)), m_optional(std::move(section.m_optional)),
    m_identifier(std::move(section.m_identifier)) {
    for (auto &value : m_values) {
        auto &current_value = value.second;
        if (std::holds_alternative<confuse_element>(current_value)) {
            std::get<confuse_element>(current_value).parent(this);
        } else if (std::holds_alternative<confuse_section>(current_value)) {
            std::get<confuse_section>(current_value).parent(this);
        }
    }
}

const std::string &confuse_section::identifier() const {
    return m_identifier;
}

// TODO set flags
cfg_opt_t confuse_section::get_confuse_representation(option_storage &opt_storage) const {
    opt_storage.emplace_back(std::make_unique<cfg_opt_t[]>(m_values.size() + 1));
    size_t storage_entry = opt_storage.size() - 1;
    size_t index = 0;

    for (const auto &value : m_values) {
        if (std::holds_alternative<confuse_section>(value.second)) {
            const confuse_section &section = std::get<confuse_section>(value.second);
            opt_storage[storage_entry][index] = section.get_confuse_representation(opt_storage);
        } else if (std::holds_alternative<confuse_element>(value.second)) {
            const confuse_element &element = std::get<confuse_element>(value.second);
            opt_storage[storage_entry][index] = element.get_confuse_representation();
        }

        ++index;
    }

    opt_storage[storage_entry][index] = CFG_END();

    cfg_opt_t ret = CFG_SEC(m_identifier.c_str(), opt_storage[storage_entry].get(), CFGF_NONE);

    return ret;
}

void confuse_section::parent(const confuse_section *parent) {
    m_parent = parent;
}

const confuse_section *confuse_section::parent() const {
    return m_parent;
}

cfg_t *confuse_section::section_handle() const {
    return cfg_getsec(m_parent->section_handle(), m_identifier.c_str());
}

// Some sort of safety so no exception gets thrown
const confuse_section::variant_type &confuse_section::operator[](const std::string &identifier) const {
    return m_values.at(identifier);
}

confuse_root::confuse_root(const std::initializer_list<variant_type> &values)
    : confuse_section("", values, false) {
}

confuse_root::confuse_root(const confuse_root &root) : confuse_section(root),
    m_section_handle(root.m_section_handle) {
}

confuse_root::confuse_root(confuse_root &&root) : confuse_section(std::move(root)),
    m_section_handle(root.m_section_handle) {
}

void confuse_root::config_handle(cfg_t *config_handle) {
    m_section_handle = config_handle;
}

cfg_t *confuse_root::section_handle() const {
    return m_section_handle;
}
