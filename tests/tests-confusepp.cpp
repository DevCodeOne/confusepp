#include <experimental/filesystem>

#include <cfloat>

#include "catch.hpp"

#include "confusepp.h"

using std::experimental::filesystem::path;

// TODO Tests with false arguments
// TODO Tests boolean List and default_value
// TODO remove unnecessary whitespaces in string for option name
TEST_CASE("confusepp") {
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
        Section("capital_of_states_in_germany")
            .values(Option<std::string>("Baden-Württemberg"), Option<std::string>("Bavaria"),
                    Option<std::string>("Berlin"), Option<std::string>("Brandenburg"), Option<std::string>("Bremen"),
                    Option<std::string>("Hamburg"), Option<std::string>("Hesse"), Option<std::string>("Lower Saxony"),
                    Option<std::string>("Mecklenburg-Vorpommern"), Option<std::string>("North Rhine-Westphalia"),
                    Option<std::string>("Rhineland-Palatinate"), Option<std::string>("Saarland"),
                    Option<std::string>("Saxony"), Option<std::string>("Saxony-Anhalt"),
                    Option<std::string>("Schleswig-Holstein"), Option<std::string>("Thuringia")),
        Multisection("person").values(Option<std::string>("firstname"), Option<std::string>("lastname"),
                                      Option<bool>("male"), Option<int>("age"),
                                      Option<float>("constant").default_value(.0f))};

    auto config = Config::parse("tests/tests.conf", root);

    SECTION("Config is Valid") { REQUIRE(config.has_value()); }

    SECTION("Options") {
        SECTION("string Option target") {
            std::string s = config->get<Option<std::string>>("target")->value();
            REQUIRE(s == "Neighbour");
        }

        SECTION("string Option name") {
            std::string fn = config->get<Option<std::string>>("firstname")->value();
            REQUIRE(fn == "Hans");

            auto ln = config->get<Option<std::string>>("lastname")->value();
            REQUIRE(ln == "Meier");
        }

        SECTION("int Option repeat") {
            auto b = config->get<Option<int>>("repeat")->value();
            REQUIRE(b == 3);
        }

        SECTION("int Option default") {
            auto b = config->get<Option<int>>("age")->value();
            REQUIRE(b == 0);  // TODO check if valid
        }
    }

    SECTION("Lists") {
        SECTION("int List lotto_numbers") {
            auto int_list = config->get<Option<List<int>>>("lotto_numbers")->value();
            REQUIRE(int_list.size() == 6);
            REQUIRE(int_list[0] == 4);
            REQUIRE(int_list[1] == 8);
            REQUIRE(int_list[2] == 15);
            REQUIRE(int_list[3] == 16);
            REQUIRE(int_list[4] == 23);
            REQUIRE(int_list[5] == 42);
        }

        SECTION("float List irrational_numbers") {
            auto float_list = config->get<Option<List<float>>>("irrational_numbers")->value();
            REQUIRE(float_list.size() == 3);
            REQUIRE(float_list[0] - 3.14159265359f <= FLT_EPSILON);
            REQUIRE(float_list[1] - 1.41421356237f <= FLT_EPSILON);
            REQUIRE(float_list[2] - 2.71828182845f <= FLT_EPSILON);
        }

        SECTION("bool List") {
            auto boolean_list = config->get<Option<List<bool>>>("a_boolean_list")->value();
            REQUIRE(boolean_list.size() == 5);
            REQUIRE(boolean_list[0] == true);
            REQUIRE(boolean_list[1] == false);
            REQUIRE(boolean_list[2] == false);
            REQUIRE(boolean_list[3] == true);
            REQUIRE(boolean_list[4] == false);
        }

        SECTION("string List presidents") {
            auto presidents = config->get<Option<List<std::string>>>("presidents")->value();
            REQUIRE(presidents.size() == 5);
            REQUIRE(presidents[0] == "George Washington");
            REQUIRE(presidents[1] == "John Adams");
            REQUIRE(presidents[2] == "Thomas Jefferson");
            REQUIRE(presidents[3] == "James Madison");
            REQUIRE(presidents[4] == "James Monroe");
        }

        SECTION("string List empty_string_list") {
            auto string_list = config->get<Option<List<std::string>>>("empty_string_list")->value();
            REQUIRE(string_list.size() == 1);
            REQUIRE(string_list[0] == "I am empty");
        }

        SECTION("string List list with no default") {
            auto string_list = config->get<Option<List<std::string>>>("list with no default")->value();
            REQUIRE(string_list.size() == 0);
        }
    }

    SECTION("Section") {
        path p("capital_of_states_in_germany");

        auto capitals = *config->get<Section>(p);

        REQUIRE(config->get<Option<std::string>>(p / "Baden-Württemberg")->value() == "Stuttgart");
        REQUIRE(config->get<Option<std::string>>(p / "Bavaria")->value() == "Munich");
        REQUIRE(config->get<Option<std::string>>(p / "Berlin")->value() == "Berlin");
        REQUIRE(config->get<Option<std::string>>(p / "Brandenburg")->value() == "Potsdam");
        REQUIRE(config->get<Option<std::string>>(p / "Bremen")->value() == "Bremen");
        REQUIRE(config->get<Option<std::string>>(p / "Hamburg")->value() == "Hamburg");
        REQUIRE(config->get<Option<std::string>>(p / "Hesse")->value() == "Wiesbaden");
        REQUIRE(config->get<Option<std::string>>(p / "Lower Saxony")->value() == "Hanover");
        REQUIRE(config->get<Option<std::string>>(p / "Mecklenburg-Vorpommern")->value() == "Schwerin");
        REQUIRE(config->get<Option<std::string>>(p / "North Rhine-Westphalia")->value() == "Düsseldorf");
        REQUIRE(config->get<Option<std::string>>(p / "Rhineland-Palatinate")->value() == "Mainz");
        REQUIRE(config->get<Option<std::string>>(p / "Saarland")->value() == "Saarbrücken");
        REQUIRE(config->get<Option<std::string>>(p / "Saxony")->value() == "Dresden");
        REQUIRE(config->get<Option<std::string>>(p / "Saxony-Anhalt")->value() == "Magdeburg");
        REQUIRE(config->get<Option<std::string>>(p / "Schleswig-Holstein")->value() == "Kiel");
        REQUIRE(config->get<Option<std::string>>(p / "Thuringia")->value() == "Erfurt");
    }

    SECTION("Multisection") {
        path p("person/euler");
        auto person = config->get<Section>("person/euler");

        // REQUIRE(person);
        REQUIRE(person->get<Option<std::string>>("firstname")->value() == "Leonhard");
        REQUIRE(person->get<Option<std::string>>("lastname")->value() == "Euler");
        REQUIRE(person->get<Option<bool>>("male")->value() == true);
        REQUIRE(person->get<Option<int>>("age")->value() == 76);
        REQUIRE(person->get<Option<float>>("constant")->value() - 2.71828182845F <= FLT_EPSILON);

        auto person2 = config->get<Section>("person/turing");

        REQUIRE(person2->get<Option<std::string>>("firstname")->value() == "Alan");
        REQUIRE(person2->get<Option<std::string>>("lastname")->value() == "Turing");
        REQUIRE(person2->get<Option<bool>>("male")->value() == true);
        REQUIRE(person2->get<Option<int>>("age")->value() == 41);
        REQUIRE(person2->get<Option<float>>("constant")->value() == 0);
    }
}
