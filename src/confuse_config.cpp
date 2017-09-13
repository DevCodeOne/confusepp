#include <iostream>

#include "confuse_config.h"

// TODO improve how fclose gets called with raii
std::optional<confuse_config> confuse_config::parse_config(const std::string &filename, confuse_root &root) {
    FILE *config_file = fopen(filename.c_str(), "r");

    if (config_file) {
        confuse_config config(nullptr);
        cfg_t *config_handle = cfg_init(root.get_confuse_representation(config.m_opt_storage).subopts, CFGF_NONE);

        if (config_handle
                && cfg_parse_fp(config_handle, config_file) == CFG_SUCCESS) {

            fclose(config_file);

            return std::move(config);
        }
    }

    fclose(config_file);

    return {};
}

confuse_config::confuse_config(cfg_t *config_handle)
    : m_config_handle(config_handle) {
}

confuse_config::confuse_config(confuse_config &&config)
    : m_config_handle(std::move(config.m_config_handle)),
    m_opt_storage(std::move(config.m_opt_storage)) {
}

confuse_config::~confuse_config() {
    cfg_free(m_config_handle);
}

const cfg_t *confuse_config::config_handle() const {
    return m_config_handle;
}

cfg_t *confuse_config::config_handle() {
    return m_config_handle;
}
