#include <iostream>

#include "confuse_elements.h"
#include "confuse_config.h"

int main() {
    auto root =
        confuse_root {
            confuse_value<int>("int_value"),
            confuse_section("string_section", {
                confuse_value<std::string>("string_identifier", "test_string")
                }),
            confuse_section("int_section", {
                confuse_value<int>("int_identifier")
                })
            };

    auto config = confuse_config::parse_config("testconf/test.conf", root);

    if (config) {
        // config is valid
        // read values
        // change some values
        std::cout << "Config is valid" << std::endl;
    }
}
