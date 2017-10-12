#include <type_traits>

#include <confuse.h>

#include "elements.h"

namespace confusepp {

    Element::Element(const std::string& identifier) : m_identifier(identifier) {}

    const std::string& Element::identifier() const { return m_identifier; }

    Section::Section(const std::string& identifier) : Element(identifier) {}

    std::optional<Section::variant_type> Section::operator[](const std::string& identifier) const {
        if (auto section = m_values.find(identifier); section != m_values.cend()) {
            return {section->second};
        }
        return {};
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

                    if constexpr (std::is_same_v<Section, current_type> || std::is_same_v<Multisection, current_type>) {
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

        auto flags = CFGF_NONE;

        if (m_title != ""s) {
            flags = CFGF_TITLE;
        }

        cfg_opt_t ret = CFG_SEC(identifier().c_str(), opt_storage[storage_entry].get(), flags);
        return ret;
    }

    Section& Section::title(const std::string& title) {
        m_title = title;
        return *this;
    }

    const std::string& Section::title() const { return m_title; }

    void Section::add_children(std::vector<variant_type> values) {
        for (auto& current_value : values) {
            std::visit(
                [this](auto& argument) {
                    auto created_value(std::move(argument));
                    m_values.emplace(argument.identifier(), created_value);
                },
                current_value);
        }
    }

    void Section::load(cfg_t* parent_handle) {
        using namespace std::string_literals;

        if (!parent_handle) {
            return;
        }

        cfg_t* current_handle = nullptr;

        if (m_title == ""s) {
            current_handle = cfg_getsec(parent_handle, identifier().c_str());
        } else {
            current_handle = cfg_gettsec(parent_handle, identifier().c_str(), title().c_str());
        }

        for (auto& current : m_values) {
            std::visit([&current_handle](auto& argument) { argument.load(current_handle); }, current.second);
        }
    }

    ConfigFormat::ConfigFormat(const std::initializer_list<variant_type>& value_list) : Section("") {
        values(value_list);
    }

    void ConfigFormat::load(cfg_t* parent_handle) {
        for (auto& current : m_values) {
            std::visit([&parent_handle](auto& argument) { argument.load(parent_handle); }, current.second);
        }
    }

    Multisection::Multisection(const std::string& identifier) : Element(identifier) {}

    std::optional<Section> Multisection::operator[](const std::string& title) const {
        if (auto section = m_sections.find(title); section != m_sections.cend()) {
            return {section->second};
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

                    if constexpr (std::is_base_of_v<Section, current_type> ||
                                  std::is_base_of_v<Multisection, current_type>) {
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

        cfg_opt_t ret = CFG_SEC(identifier().c_str(), opt_storage[storage_entry].get(), CFGF_MULTI | CFGF_TITLE);
        return ret;
    }

    void Multisection::load(cfg_t* parent_handle) {
        size_t number_of_sections = cfg_size(parent_handle, identifier().c_str());

        for (size_t i = 0; i < number_of_sections; i++) {
            cfg_t* sub_section_handle = cfg_getnsec(parent_handle, identifier().c_str(), i);
            const char* sub_section_title = sub_section_handle->title;

            auto created_section = m_sections.emplace(
                std::make_pair(sub_section_title, Section(identifier()).title(sub_section_title).values(m_values)));

            created_section.first->second.load(parent_handle);
        }
    }
}  // namespace confusepp
