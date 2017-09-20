#include <iostream>

#include "confuse_elements.h"
#include "confuse_config.h"

int main() {
    auto root =
        confuse_root {
            confuse_value<int>("int_value", 42),
            confuse_section("string_section", {
                    confuse_value<std::string>("string_identifier", "test_string")
                    }),
            confuse_section("int_section", {
                    confuse_value<int>("int_identifier")
                    })
        };

    auto config = confuse_config::parse_config("tests/test.conf", std::move(root));

    if (config) {
        std::cout << "Config is valid" << std::endl;

        auto &root_node = config->root_node();
        std::cout << std::get<int>(std::get<confuse_element>(root_node["int_value"]).value()) << std::endl;

        auto &string_section = std::get<confuse_section>(root_node["string_section"]);

        std::cout << std::get<std::string>(std::get<confuse_element>(string_section["string_identifier"]).value()) << std::endl;

    }
}
