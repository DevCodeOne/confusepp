#pragma once

#include <memory>
#include <optional>
#include <type_traits>
#include <vector>

#include "elements.h"

namespace confusepp {
    /**
     * @brief The Config class which provides the content of the ConfigFile
     */
    class Config {
       public:
        Config(Config& config) = delete; /**< no copy constructable allowed */
        Config(Config&& config); /**< move constructable allowed */
        ~Config();

        Config& operator=(const Config& config) = delete; /**< no copy asignment */
        Config& operator=(Config&& config) = delete; /**< no move asignment */

        /**
         * @brief parse-method which creats the config_tree from the config_file
         * @param config_file File which provides the config
         * @param root root-element of the config_tree
         * @return Empty or filled Config-Instance
         */
        static std::optional<Config> parse(const std::experimental::filesystem::path& config_file, ConfigFormat root);
        template<typename T>
        std::optional<T> get(const std::experimental::filesystem::path& element_path) const;

       private:
        Config(ConfigFormat config_tree, cfg_t* config_handle = nullptr);
        void config_handle(cfg_t* handle);

        bool m_valid = false;
        cfg_t* m_config_handle;
        ConfigFormat m_config_tree;
        std::vector<std::unique_ptr<cfg_opt_t[]>> m_opt_storage;
    };

    template<typename T>
    std::optional<T> Config::get(const std::experimental::filesystem::path& element_path) const {
        const Section& sec(m_config_tree);
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
