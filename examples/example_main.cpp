#include <iostream>

#include "confusepp.h"

int main() {
    using namespace confuse;
    Root root{Option<int>("int_value").default_value(0),
              Section("string_section").values(Option<std::string>("string_identifier").default_value("test_string")),
              Section("int_section")
                  .values(Option<int>("int_one"), Option<int>("int_two"), Option<int>("int_three"))
                  .title("test"),
              Multisection("multi").values(Option<std::string>("string_identifier")),
              Option<List<int>>("int_list").default_value(42, 13),
              Option<List<float>>("float_list"),
              Option<List<bool>>("bool_list"),
              Option<List<std::string>>("string_list").default_value("test, test", "test2")};

    auto config = Config::parse_config("examples/test.conf", root);

    if (config) {
        std::cout << "Config is valid." << std::endl;

        auto &root_node = config->root_node();
        auto element = root_node.get<Option<int>>("int_value");
        std::cout << element.value() << std::endl;

        auto &string_section = root_node.get<Section>("string_section");
        std::cout << string_section.get<Option<std::string>>("string_identifier").value() << std::endl;

        auto multi_title = root_node.get<Multisection>("multi")["title"];

        if (multi_title) {
            std::cout << multi_title->get<Option<std::string>>("string_identifier").value() << std::endl;
        }

        auto &string_data = root_node.get<Option<List<std::string>>>("string_list").value();

        for (const auto &current_value : string_data) {
            std::cout << current_value << " ";
        }
        std::cout << std::endl;
    } else {
        std::cerr << "Config is invalid. " << std::endl;
    }
}
