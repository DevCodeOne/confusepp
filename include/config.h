#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "elements.h"

namespace confusepp {

    class Config {
       public:
        Config(Config& config) = delete;
        Config(Config&& config);
        ~Config();

        Config& operator=(const Config& config) = default;
        Config& operator=(Config&& config) = default;

        static std::optional<Config> parse_config(const std::string& filename, ConfigFormat root);
        const ConfigFormat& root_node() const;

       private:
        Config(ConfigFormat config_tree, cfg_t* config_handle = nullptr);
        void config_handle(cfg_t* config_handle);
        const cfg_t* config_handle() const;
        cfg_t* config_handle();

        cfg_t* m_config_handle;
        ConfigFormat m_config_tree;
        std::vector<std::unique_ptr<cfg_opt_t[]>> m_opt_storage;
    };
}  // namespace confusepp
