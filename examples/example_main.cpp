#include <iostream>

#include "confusepp.h"

int main() {
    confuse_root root{
            confuse_value<int>("int_value").default_value(0),
            confuse_section("string_section", {
                    confuse_value<std::string>("string_identifier").default_value("test_string")
                    }),
            confuse_section("int_section", {
                    confuse_value<int>("int_identifier"),
                    }).title("test"),
            confuse_multi_section("multi", {
                    confuse_value<std::string>("string_identifier"),
                    }),
            confuse_value<confuse_list<int>>("int_list").default_value(42, 13),
            confuse_value<confuse_list<float>>("float_list"),
            confuse_value<confuse_list<bool>>("bool_list"),
            confuse_value<confuse_list<std::string>>("string_list").default_value("test, test", "test2")
        };

    auto config = confuse_config::parse_config("tests/test.conf", root);

    if (config) {
        std::cout << "Config is valid" << std::endl;

        auto &root_node = config->root_node();
        auto element = root_node.get<confuse_value<int>>("int_value");
        std::cout << element.value() << std::endl;

        auto &string_section = root_node.get<confuse_section>("string_section");
        std::cout << string_section.get<confuse_value<std::string>>("string_identifier").value() << std::endl;

        auto multi_title = root_node.get<confuse_multi_section>("multi")["title"];

        if (multi_title) {
            std::cout << multi_title->get<confuse_value<std::string>>("string_identifier").value() << std::endl;
        }

        auto &string_data = root_node.get<confuse_value<confuse_list<std::string>>>("string_list").value();

        for (const auto &current_value : string_data) {
            std::cout << current_value << " ";
        }
        std::cout << std::endl;
    }
}
