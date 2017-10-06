#include <iostream>
#include <cfloat>
#include "catch.hpp"
#include "confusepp.h"

//TODO Tests with false arguments
//TODO Tests boolean List and default_value
TEST_CASE("confusepp"){
    using namespace confusepp;
    ConfigFormat root{
            Option<List<int>>("lotto_numbers").default_value(42),
            Option<std::string>("target").default_value("World"),
            Option<std::string>("firstname").default_value("Hans"),
            Option<List<float>>("irrational_numbers"),
            Option<std::string>("lastname").default_value("Müller"),
            Option<int>("repeat").default_value(13),
            Option<int>("age"),
            Option<List<bool>>("a_boolean_list"),
            Option<List<std::string>>("presidents").default_value("Abraham Lincoln"),
            Option<List<std::string>>("empty_string_list").default_value("I am empty"),
            Option<List<std::string>>("list with no default"),
    };

    auto config = Config::parse_config("tests/tests.conf", root);

    SECTION("Config is Valid") {
        REQUIRE(config.has_value());
    }

    auto &root_node = config->root_node();
    SECTION("Options"){
        SECTION("string Option target") {
            std::string s = root_node.get<Option<std::string>>("target").value();
            REQUIRE(s == "Neighbour");
        }

        SECTION("string Option name") {
            std::string fn = root_node.get<Option<std::string>>("firstname").value();
            REQUIRE(fn == "Hans");

            auto ln = root_node.get<Option<std::string>>("lastname").value();
            REQUIRE(ln == "Meier");
        }

        SECTION("int Option repeat") {
            auto b = root_node.get<Option<int>>("repeat").value();
            REQUIRE(b == 3);
        }

        SECTION("int Option default") {
            auto b = root_node.get<Option<int>>("age").value();
            REQUIRE(b == 0); //TODO check if valid
        }
    }

    SECTION("Lists") {
        SECTION("int List lotto_numbers") {
            auto& int_list = root_node.get<Option<List<int>>>("lotto_numbers").value();
            REQUIRE(int_list.size() == 6);
            REQUIRE(int_list[0] == 4);
            REQUIRE(int_list[1] == 8);
            REQUIRE(int_list[2] == 15);
            REQUIRE(int_list[3] == 16);
            REQUIRE(int_list[4] == 23);
            REQUIRE(int_list[5] == 42);
        }

        SECTION("float List irrational_numbers") {
            auto& float_list = root_node.get<Option<List<float>>>("irrational_numbers").value();
            REQUIRE(float_list.size() == 3);
            REQUIRE(float_list[0] - 3.14159265359f <= FLT_EPSILON);
            REQUIRE(float_list[1] - 1.41421356237f <= FLT_EPSILON);
            REQUIRE(float_list[2] - 2.71828182845f <= FLT_EPSILON);
        }

        SECTION("bool List") {
            auto& boolean_list = root_node.get<Option<List<bool>>>("a_boolean_list").value();
            REQUIRE(boolean_list.size() == 5);
            REQUIRE(boolean_list[0] == true);
            REQUIRE(boolean_list[1] == false);
            REQUIRE(boolean_list[2] == false);
            REQUIRE(boolean_list[3] == true);
            REQUIRE(boolean_list[4] == false);
        }

        SECTION("string List presidents") {
            auto& presidents = root_node.get<Option<List<std::string>>>("presidents").value();
            REQUIRE(presidents.size() == 5);
            REQUIRE(presidents[0] == "George Washington");
            REQUIRE(presidents[1] == "John Adams");
            REQUIRE(presidents[2] == "Thomas Jefferson");
            REQUIRE(presidents[3] == "James Madison");
            REQUIRE(presidents[4] == "James Monroe");
        }

        SECTION("string List empty_string_list") {
            auto& string_list = root_node.get<Option<List<std::string>>>("empty_string_list").value();
            REQUIRE(string_list.size() == 1);
            REQUIRE(string_list[0] == "I am empty");
        }

        SECTION("string List list with no default") {
            auto& string_list = root_node.get<Option<List<std::string>>>("list with no default").value();
            REQUIRE(string_list.size() == 0);
        }

    }
}
/*
TEST_CASE("Confusepp"){
    using namespace confusepp;
    ConfigFormat root{Option<int>("int_value").default_value(0),
              Section("string_section").values(Option<std::string>("string_identifier").default_value("test_string")),
              Section("int_section").values(
                      Option<int>("int_one"),
                      Option<int>("int_two"),
                      Option<int>("int_three")
              ).title("test"),
              Multisection("multi").values(Option<std::string>("string_identifier")),
              Option<List<int>>("int_list").default_value(42, 13),
              Option<List<float>>("float_list"),
              Option<List<bool>>("bool_list"),
              Option<List<std::string>>("string_list").default_value("test, test", "test2")};

    auto config = Config::parse_config("tests/tests.conf", root);

    SECTION("Config is Valid") {
        REQUIRE(config.has_value());
    }

    auto &root_node = config->root_node();

    SECTION("int Option") {
        int i = root_node.get<Option<int>>("int_value").value();
        REQUIRE(i == 10);
    }

    SECTION("string Section") {
        auto& string_section = root_node.get<Section>("string_section");
        std::string s =  string_section.get<Option<std::string>>("string_identifier").value();
        REQUIRE(s == "Hello world");
    }

    SECTION("multi title") {
        auto multi_title = root_node.get<Multisection>("multi")["title"];
        REQUIRE(multi_title.has_value());
        auto multi = multi_title->get<Option<std::string>>("string_identifier").value();
        REQUIRE(multi)
            std::cout << multi_title->get<Option<std::string>>("string_identifier").value() << std::endl;

    }
    auto &string_data = root_node.get<Option<List<std::string>>>("string_list").value();

    for (const auto &current_value : string_data) {
        std::cout << current_value << " ";
    }
    std::cout << std::endl;
}*/
