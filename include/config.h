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
    class Config final {
       public:
        Config(Config& config) = delete; /**< no copy constructable allowed */
        Config(Config&& config);         /**< move constructable allowed */
        ~Config();

        Config& operator=(const Config& config) = delete; /**< no copy asignment */
        Config& operator=(Config&& config) = delete;      /**< no move asignment */

        /**
         * @brief Parse-method which creats the config_tree from the config_file
         * @param config_file File which provides the config
         * @param root root-element of the config_tree
         * @return Empty or filled Config-Instance
         */
        static std::optional<Config> parse(const path& config_file, ConfigFormat root);

        template<typename T>
        /**
         * @brief get the Element at the specified path
         * @param element_path of the element
         * @return Element at the specified path
         */
        std::optional<T> get(const path& element_path) const;

       private:
        /**
         * @brief Constuct the Config with the
         * @param config_tree Tree represantation of the config
         * @param config_handle the confuse handle for the root section
         */
        Config(ConfigFormat config_tree, cfg_t* config_handle = nullptr);

        /**
         * @brief config_handle Initialize the config-tree
         * @param handle root handle from confuse
         */
        void config_handle(cfg_t* handle);

        /**
         * @brief m_valid runtime check for config tree
         */
        bool m_valid = false;
        cfg_t* m_config_handle;
        ConfigFormat m_config_tree;
        /**
         * @brief m_opt_storage Storage for the confuse representation
         */
        std::vector<std::unique_ptr<cfg_opt_t[]>> m_opt_storage;
    };

    template<typename T>
    std::optional<T> Config::get(const path& element_path) const {
        return m_config_tree.get<T>(element_path);
    }
}  // namespace confusepp
