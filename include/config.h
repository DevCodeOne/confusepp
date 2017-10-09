#pragma once

#include <memory>
#include <optional>
#include <type_traits>
#include <vector>

#include "elements.h"

namespace confusepp {

    class Config {
       public:
        Config(Config& config) = delete;
        Config(Config&& config);
        ~Config();

        Config& operator=(const Config& config) = delete;
        Config& operator=(Config&& config) = delete;

        static std::optional<Config> parse(const std::experimental::filesystem::path& config_file, ConfigFormat root);
        template<typename T>
        std::optional<T> get(const std::experimental::filesystem::path& element_path);

       private:
        Config(ConfigFormat config_tree, cfg_t* config_handle = nullptr);
        void config_handle(cfg_t* handle);

        bool m_valid = false;
        cfg_t* m_config_handle;
        ConfigFormat m_config_tree;
        std::vector<std::unique_ptr<cfg_opt_t[]>> m_opt_storage;
    };

    template<typename T>
    std::optional<T> Config::get(const std::experimental::filesystem::path& element_path) {
        Section& sec(m_config_tree);
        std::optional<ConfigFormat::variant_type> current(sec);

        for (auto it = element_path.begin(); it != element_path.end(); ++it) {
            if (current) {
                std::visit(
                    [&](auto& parent_element) {
                        using current_type = std::decay_t<decltype(parent_element)>;

                        if constexpr (std::is_same_v<Section, current_type> ||
                                      std::is_same_v<Multisection, current_type>) {
                            auto next = parent_element[std::string(it->c_str())];

                            if (next) {
                                current = *next;
                            } else {
                                current = std::optional<T>{};
                            }
                        } else {
                            current = std::optional<T>{};
                        }
                    },
                    *current);
            }
            if (!current) {
                return std::optional<T>{};
            }
        }

        if (std::holds_alternative<T>(*current)) {
            return std::get<T>(*current);
        }

        return std::optional<T>{};
    }
}  // namespace confusepp
