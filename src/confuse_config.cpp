#include <memory>

#include "confuse_config.h"

// also create different version that take confuse_root as rvalue
// (in this method just create copy and cast the copy to rvalue and call other func)
std::optional<confuse_config> confuse_config::parse_config(const std::string &filename, confuse_root root) {
    std::unique_ptr<FILE, decltype(&std::fclose)> config_file(std::fopen(filename.c_str(), "r"), &std::fclose);

    if (config_file) {
        confuse_config config(std::move(root));
        cfg_opt_t config_structure = config.root_node().get_confuse_representation(config.m_opt_storage);
        cfg_t *config_handle = cfg_init(config_structure.subopts, CFGF_NONE);

        if (config_handle && cfg_parse_fp(config_handle, config_file.get()) == CFG_SUCCESS) {
            config.config_handle(config_handle);
            return std::optional<confuse_config>{std::move(config)};
        }
    }

    return std::optional<confuse_config>{};
}

confuse_config::confuse_config(confuse_root config_tree, cfg_t *config_handle)
    : m_config_handle(config_handle), m_config_tree(std::move(config_tree)) {
}

confuse_config::confuse_config(confuse_config &&config)
    : m_config_handle(std::move(config.m_config_handle)),
    m_config_tree(std::move(config.m_config_tree)),
    m_opt_storage(std::move(config.m_opt_storage)) {
    config.m_config_handle = nullptr;
}

confuse_config::~confuse_config() {
    if (m_config_handle) {
        cfg_free(m_config_handle);
    }
}

void confuse_config::config_handle(cfg_t *config_handle) {
    m_config_handle = config_handle;
    m_config_tree.config_handle(m_config_handle);
}

const confuse_root &confuse_config::root_node() const {
    return m_config_tree;
}

const cfg_t *confuse_config::config_handle() const {
    return m_config_handle;
}

cfg_t *confuse_config::config_handle() {
    return m_config_handle;
}
