#include <iostream>

#include "confusepp.h"

int main() {
    auto root =
        confuse_root {
            confuse_value<int>("int_value", 21),
            confuse_section("string_section", {
                    confuse_value<std::string>("string_identifier", "test_string")
                    }),
            confuse_section("int_section", {
                    confuse_value<int>("int_identifier")
                    }),
            confuse_value<confuse_list<int>>("int_list", confuse_list<int>({42, 13})),
            confuse_value<confuse_list<float>>("float_list"),
            confuse_value<confuse_list<bool>>("bool_list"),
            confuse_value<confuse_list<std::string>>("string_list")
        };

    auto config = confuse_config::parse_config("tests/test.conf", std::move(root));

    if (config) {
        std::cout << "Config is valid" << std::endl;

        auto &root_node = config->root_node();
        auto element = root_node.get<confuse_value<int>>("int_value");
        std::cout << element.value() << std::endl;

        auto &string_section = root_node.get<confuse_section>("string_section");
        std::cout << string_section.get<confuse_value<std::string>>("string_identifier").value() << std::endl;

        auto &int_data = root_node.get<confuse_value<confuse_list<int>>>("int_list").value();

        for (const auto &current_value : int_data) {
            std::cout << current_value << " ";
        }
        std::cout << std::endl;
    }
}
