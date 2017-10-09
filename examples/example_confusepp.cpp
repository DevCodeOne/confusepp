#include <iostream>

#include "confusepp.h"

using std::experimental::filesystem::path;

int main() {
    using namespace confusepp;

    ConfigFormat format{
        Option<int>("int_value").default_value(0),
        Option<List<bool>>("bool_list").default_value(true, false),
        Section("string_section").values(Option<std::string>("string_identifier").default_value("test_string")),
        Multisection("int_section")
            .values(Option<int>("int_one").default_value(13), Option<int>("int_two"), Option<int>("int_three")),
        Multisection("multi").values(Option<std::string>("string_identifier")),
        Option<List<int>>("int_list").default_value(42, 13),
        Option<List<float>>("float_list"),
        Option<List<bool>>("bool_list"),
        Option<List<std::string>>("string_list").default_value("test, test", "test2")};

    auto config = Config::parse("examples/example.conf", format);

    if (config) {
        std::cout << "Config is valid." << std::endl;

        if (auto value = config->get<Option<int>>("int_value")) {
            std::cout << value->value() << std::endl;
        }

        if (auto value = config->get<Section>("multi/title")) {
            std::cout << value->get<Option<std::string>>("string_identifier")->value() << std::endl;
        }

        path p("int_section");

        p /= "test";
        p /= "int_one";

        if (auto value = config->get<Option<int>>(p)) {
            std::cout << value->value() << std::endl;
        }

        if (auto value = config->get<Option<int>>(p / "test")) {
            std::cout << value->value() << std::endl;
        } else {
            std::cout << "Wrong path but no error" << std::endl;
        }

        if (auto value = config->get<Option<List<int>>>("int_list")) {
            for (auto &current_val : value->value()) {
                std::cout << current_val << " ";
            }
            std::cout << std::endl;
        }
    } else {
        std::cerr << "Config is invalid. " << std::endl;
    }
}
