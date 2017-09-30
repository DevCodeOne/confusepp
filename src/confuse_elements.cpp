#include <type_traits>

#include <confuse.h>

#include "confuse_elements.h"

confuse_element::confuse_element(const std::string &identifier)
    : m_identifier(identifier) {
}

confuse_element::confuse_element(const confuse_element &element)
    : m_parent(element.m_parent), m_identifier(element.m_identifier) {
}

confuse_element::confuse_element(confuse_element &&element)
    : m_parent(std::move(element.m_parent)), m_identifier(std::move(element.m_identifier)) {
    element.m_parent = nullptr;
}

const std::string &confuse_element::identifier() const {
    return m_identifier;
}

void confuse_element::parent(const confuse_section *parent) {
    m_parent = parent;
}

const confuse_section *confuse_element::parent() const {
    return m_parent;
}

confuse_section::confuse_section(const std::string &identifier, const std::initializer_list<variant_type> &values)
    : confuse_element(identifier) {
    for (auto &current_value : values) {
        std::visit([this](auto &argument) {
                    auto created_value(argument);
                    created_value.parent(this);
                    m_values.emplace(argument.identifier(), created_value);
                }, current_value);
    }
}

confuse_section::confuse_section(const confuse_section &section)
    : confuse_element(section), m_values(section.m_values) {
    for (auto &current_value : m_values) {
        std::visit([this](auto &argument) { argument.parent(this); }, current_value.second);
    }
}

confuse_section::confuse_section(confuse_section &&section)
    : confuse_element(section), m_values(std::move(section.m_values)) {
    for (auto &current_value : m_values) {
        std::visit([this](auto &argument) { argument.parent(this); }, current_value.second);
    }
}

// TODO set flags
cfg_opt_t confuse_section::get_confuse_representation(option_storage &opt_storage) const {
    opt_storage.emplace_back(std::make_unique<cfg_opt_t[]>(m_values.size() + 1));
    size_t storage_entry = opt_storage.size() - 1;
    size_t index = 0;

    for (const auto &current_value : m_values) {
        cfg_opt_t opt_definition;

        std::visit([&opt_definition, &opt_storage](auto &argument) {
                    using current_type = std::decay_t<decltype(argument)>;

                    if constexpr (std::is_same_v<current_type, confuse_section>) {
                        opt_definition =  argument.get_confuse_representation(opt_storage);
                    } else {
                        opt_definition =  argument.get_confuse_representation();
                    }
                }, current_value.second);

        opt_storage[storage_entry][index] = opt_definition;
        ++index;
    }

    opt_storage[storage_entry][index] = CFG_END();

    cfg_opt_t ret = CFG_SEC(m_identifier.c_str(), opt_storage[storage_entry].get(), CFGF_NONE);
    return ret;
}

cfg_t *confuse_section::section_handle() const {
    return cfg_getsec(m_parent->section_handle(), m_identifier.c_str());
}

const confuse_section::variant_type &confuse_section::operator[](const std::string &identifier) const {
    return m_values.at(identifier);
}

confuse_root::confuse_root(const std::initializer_list<variant_type> &values)
    : confuse_section("", values) {
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
