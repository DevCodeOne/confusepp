#include <type_traits>

#include <confuse.h>

#include "elements.h"

namespace confusepp {

    Element::Element(const std::string& identifier) : m_identifier(identifier) {}

    Element::Element(const Element& element) : m_parent(element.m_parent), m_identifier(element.m_identifier) {}

    Element::Element(Element&& element)
        : m_parent(std::move(element.m_parent)), m_identifier(std::move(element.m_identifier)) {
        element.m_parent = nullptr;
    }

    const std::string& Element::identifier() const { return m_identifier; }

    void Element::parent(const Parent* parent) { m_parent = parent; }

    const Parent* Element::parent() const { return m_parent; }

    Section::Section(const std::string& identifier) : Element(identifier) {}

    Section::Section(const Section& section) : Element(section), m_values(section.m_values), m_title(section.m_title) {
        for (auto& current_value : m_values) {
            std::visit([this](auto& argument) { argument.parent(this); }, current_value.second);
        }
    }

    Section::Section(Section&& section)
        : Element(section), m_values(std::move(section.m_values)), m_title(std::move(section.m_title)) {
        for (auto& current_value : m_values) {
            std::visit([this](auto& argument) { argument.parent(this); }, current_value.second);
        }
    }

    cfg_opt_t Section::get_confuse_representation(option_storage& opt_storage) const {
        using namespace std::string_literals;

        opt_storage.emplace_back(std::make_unique<cfg_opt_t[]>(m_values.size() + 1));
        size_t storage_entry = opt_storage.size() - 1;
        size_t index = 0;

        for (const auto& current_value : m_values) {
            cfg_opt_t opt_definition;

            std::visit(
                [&opt_definition, &opt_storage](auto& argument) {
                    using current_type = std::decay_t<decltype(argument)>;

                    if constexpr (std::is_base_of_v<Parent, current_type>) {
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

    cfg_t* Section::section_handle() const {
        using namespace std::string_literals;

        if (!m_parent || !m_parent->section_handle()) {
            return nullptr;
        }

        if (m_title == ""s) {
            return cfg_getsec(m_parent->section_handle(), m_identifier.c_str());
        }

        return cfg_gettsec(m_parent->section_handle(), m_identifier.c_str(), m_title.c_str());
    }

    Section& Section::title(const std::string& title) {
        m_title = title;

        return *this;
    }

    void Section::add_children(std::vector<variant_type> values) {
        for (auto& current_value : values) {
            std::visit(
                [this](auto& argument) {
                    auto created_value(std::move(argument));
                    created_value.parent(this);
                    m_values.emplace(argument.identifier(), created_value);
                },
                current_value);
        }
    }

    Root::Root(const std::initializer_list<variant_type>& value_list) : Section("") { values(value_list); }

    Root::Root(const Root& root) : Section(root), m_section_handle(root.m_section_handle) {}

    Root::Root(Root&& root) : Section(root), m_section_handle(root.m_section_handle) {}

    void Root::config_handle(cfg_t* config_handle) { m_section_handle = config_handle; }

    cfg_t* Root::section_handle() const { return m_section_handle; }

    Multisection::Multisection(const std::string& identifier) : Element(identifier) {}

    Multisection::Multisection(const Multisection& section) : Element(section), m_values(section.m_values) {
        for (auto& current_value : m_values) {
            std::visit([this](auto& argument) { argument.parent(this); }, current_value);
        }
    }

    Multisection::Multisection(Multisection&& section) : Element(section), m_values(section.m_values) {
        for (auto& current_value : m_values) {
            std::visit([this](auto& argument) { argument.parent(this); }, current_value);
        }
    }

    std::optional<Section> Multisection::operator[](const std::string& title) const {
        if (auto section = m_sections.find(title); section != m_sections.cend()) {
            return {section->second};
        }

        if (m_parent && m_parent->section_handle()) {
            cfg_t* found_section = cfg_gettsec(m_parent->section_handle(), m_identifier.c_str(), title.c_str());

            if (found_section) {
                auto value = m_sections.emplace(std::make_pair(title, Section(m_identifier).title(title)));
                value.first->second.add_children(m_values);
                value.first->second.parent(m_parent);
                return {value.first->second};
            }
        }

        return {};
    }

    cfg_opt_t Multisection::get_confuse_representation(option_storage& opt_storage) const {
        opt_storage.emplace_back(std::make_unique<cfg_opt_t[]>(m_values.size() + 1));
        size_t storage_entry = opt_storage.size() - 1;
        size_t index = 0;

        for (const auto& current_value : m_values) {
            cfg_opt_t opt_definition;

            std::visit(
                [&opt_definition, &opt_storage](auto& argument) {
                    using current_type = std::decay_t<decltype(argument)>;

                    if constexpr (std::is_base_of_v<Parent, current_type>) {
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
    cfg_t* Multisection::section_handle() const { return cfg_getsec(m_parent->section_handle(), m_identifier.c_str()); }
}  // namespace confusepp
