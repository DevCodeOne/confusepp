#include <type_traits>

#include <confuse.h>

#include "confuse_elements.h"

confuse::Element::Element(const std::string &identifier) : m_identifier(identifier) {}

confuse::Element::Element(const confuse::Element &element)
    : m_parent(element.m_parent), m_identifier(element.m_identifier) {}

confuse::Element::Element(confuse::Element &&element)
    : m_parent(std::move(element.m_parent)), m_identifier(std::move(element.m_identifier)) {
    element.m_parent = nullptr;
}

const std::string &confuse::Element::identifier() const { return m_identifier; }

void confuse::Element::parent(const confuse::Parent *parent) { m_parent = parent; }

const confuse::Parent *confuse::Element::parent() const { return m_parent; }

confuse::Section::Section(const std::string &identifier, const std::initializer_list<variant_type> &values)
    : confuse::Element(identifier) {
    add_children(values);
}

confuse::Section::Section(const confuse::Section &section)
    : confuse::Element(section), m_values(section.m_values), m_title(section.m_title) {
    for (auto &current_value : m_values) {
        std::visit([this](auto &argument) { argument.parent(this); }, current_value.second);
    }
}

confuse::Section::Section(confuse::Section &&section)
    : confuse::Element(section), m_values(std::move(section.m_values)), m_title(std::move(section.m_title)) {
    for (auto &current_value : m_values) {
        std::visit([this](auto &argument) { argument.parent(this); }, current_value.second);
    }
}

cfg_opt_t confuse::Section::get_confuse_representation(option_storage &opt_storage) const {
    using namespace std::string_literals;

    opt_storage.emplace_back(std::make_unique<cfg_opt_t[]>(m_values.size() + 1));
    size_t storage_entry = opt_storage.size() - 1;
    size_t index = 0;

    for (const auto &current_value : m_values) {
        cfg_opt_t opt_definition;

        std::visit(
            [&opt_definition, &opt_storage](auto &argument) {
                using current_type = std::decay_t<decltype(argument)>;

                if constexpr (std::is_base_of_v<confuse::Parent, current_type>) {
                    opt_definition = argument.get_confuse_representation(opt_storage);
                } else {
                    opt_definition = argument.get_confuse_representation();
                }
            },
            current_value.second);

        opt_storage[storage_entry][index] = opt_definition;
        ++index;
    }

    opt_storage[storage_entry][index] = CFG_END();

    cfg_opt_t ret =
        CFG_SEC(m_identifier.c_str(), opt_storage[storage_entry].get(), m_title == ""s ? CFGF_NONE : CFGF_TITLE);
    return ret;
}

cfg_t *confuse::Section::section_handle() const {
    using namespace std::string_literals;

    if (!m_parent || !m_parent->section_handle()) {
        return nullptr;
    }

    if (m_title == ""s) {
        return cfg_getsec(m_parent->section_handle(), m_identifier.c_str());
    }

    return cfg_gettsec(m_parent->section_handle(), m_identifier.c_str(), m_title.c_str());
}

confuse::Section &confuse::Section::title(const std::string &title) {
    m_title = title;

    return *this;
}

void confuse::Section::add_children(std::vector<variant_type> values) {
    for (auto &current_value : values) {
        std::visit(
            [this](auto &argument) {
                auto created_value(std::move(argument));
                created_value.parent(this);
                m_values.emplace(argument.identifier(), created_value);
            },
            current_value);
    }
}

confuse::Root::Root(const std::initializer_list<variant_type> &values) : confuse::Section("", values) {}

confuse::Root::Root(const confuse::Root &root) : confuse::Section(root), m_section_handle(root.m_section_handle) {}

confuse::Root::Root(confuse::Root &&root) : confuse::Section(root), m_section_handle(root.m_section_handle) {}

void confuse::Root::config_handle(cfg_t *config_handle) { m_section_handle = config_handle; }

cfg_t *confuse::Root::section_handle() const { return m_section_handle; }

confuse::Multisection::Multisection(const std::string &identifier, const std::initializer_list<variant_type> &values)
    : confuse::Element(identifier), m_values(values) {
    for (auto &current_value : m_values) {
        std::visit([this](auto &argument) { argument.parent(this); }, current_value);
    }
}

confuse::Multisection::Multisection(const confuse::Multisection &section)
    : confuse::Element(section), m_values(section.m_values) {
    for (auto &current_value : m_values) {
        std::visit([this](auto &argument) { argument.parent(this); }, current_value);
    }
}

confuse::Multisection::Multisection(confuse::Multisection &&section)
    : confuse::Element(section), m_values(section.m_values) {
    for (auto &current_value : m_values) {
        std::visit([this](auto &argument) { argument.parent(this); }, current_value);
    }
}

std::optional<confuse::Section> confuse::Multisection::operator[](const std::string &title) const {
    if (auto section = m_sections.find(title); section != m_sections.cend()) {
        return { section->second };
    }

    if (m_parent && m_parent->section_handle()) {
        cfg_t *found_section = cfg_gettsec(m_parent->section_handle(), m_identifier.c_str(), title.c_str());

        if (found_section) {
            auto value = m_sections.emplace(std::make_pair(title, confuse::Section(m_identifier, {}).title(title)));
            value.first->second.add_children(m_values);
            value.first->second.parent(m_parent);
            return { value.first->second };
        }
    }

    return {};
}

cfg_opt_t confuse::Multisection::get_confuse_representation(option_storage &opt_storage) const {
    opt_storage.emplace_back(std::make_unique<cfg_opt_t[]>(m_values.size() + 1));
    size_t storage_entry = opt_storage.size() - 1;
    size_t index = 0;

    for (const auto &current_value : m_values) {
        cfg_opt_t opt_definition;

        std::visit(
            [&opt_definition, &opt_storage](auto &argument) {
                using current_type = std::decay_t<decltype(argument)>;

                if constexpr (std::is_base_of_v<confuse::Parent, current_type>) {
                    opt_definition = argument.get_confuse_representation(opt_storage);
                } else {
                    opt_definition = argument.get_confuse_representation();
                }
            },
            current_value);

        opt_storage[storage_entry][index] = opt_definition;
        ++index;
    }

    opt_storage[storage_entry][index] = CFG_END();

    cfg_opt_t ret = CFG_SEC(m_identifier.c_str(), opt_storage[storage_entry].get(), CFGF_MULTI | CFGF_TITLE);
    return ret;
}

// TODO maybe for later multi sections in multi sections ?
cfg_t *confuse::Multisection::section_handle() const {
    return cfg_getsec(m_parent->section_handle(), m_identifier.c_str());
}
