#include <memory>

#include "config.h"

namespace confusepp {

    std::optional<Config> Config::parse(const path& config_path, ConfigFormat root) {
        std::unique_ptr<FILE, decltype(&std::fclose)> config_file(std::fopen(config_path.c_str(), "r"), &std::fclose);
        auto directory = config_path;
        directory.remove_filename();

        if (config_file) {
            Config config(std::move(root));
            cfg_opt_t config_structure = config.m_config_tree.get_confuse_representation(config.m_opt_storage);
            cfg_t* config_handle = cfg_init(config_structure.subopts, CFGF_NONE);
            cfg_add_searchpath(config_handle, directory.c_str());

            if (config_handle && cfg_parse_fp(config_handle, config_file.get()) == CFG_SUCCESS) {
                config.config_handle(config_handle);
                return std::optional<Config>{std::move(config)};
            }
        }

        return std::optional<Config>{};
    }

    Config::Config(ConfigFormat config_tree, cfg_t* config_handle)
        : m_config_handle(config_handle), m_config_tree(std::move(config_tree)) {
    }

    Config::Config(Config&& config)
        : m_config_handle(std::move(config.m_config_handle)),
          m_config_tree(std::move(config.m_config_tree)),
          m_opt_storage(std::move(config.m_opt_storage)) {
        config.m_config_handle = nullptr;
    }

    Config::~Config() {
        if (m_config_handle) {
            cfg_free(m_config_handle);
        }
    }

    void Config::config_handle(cfg_t *handle) {
        m_config_handle = handle;
        m_config_tree.load(m_config_handle);
    }

}  // namespace confusepp
