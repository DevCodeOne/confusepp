#pragma once

#include <vector>
#include <memory>
#include <optional>

#include "confuse_elements.h"

// TODO implement rule of 5
class confuse_config {
    public:
        confuse_config(confuse_config &config) = delete;
        confuse_config(confuse_config &&config);
        ~confuse_config();

        confuse_config &operator=(const confuse_config &config) = default;
        confuse_config &operator=(confuse_config &&config) = default;

        static std::optional<confuse_config> parse_config(const std::string &filename,
                confuse_root root);
        const confuse_root &root_node() const;
    private:
        confuse_config(confuse_root config_tree, cfg_t *config_handle = nullptr);
        void config_handle(cfg_t *config_handle);
        const cfg_t *config_handle() const;
        cfg_t *config_handle();

        cfg_t *m_config_handle;
        confuse_root m_config_tree;
        std::vector<std::unique_ptr<cfg_opt_t[]>> m_opt_storage;
};
