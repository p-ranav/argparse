#include <argparse/argparse.hpp>
#include <doctest.hpp>
#include <string>

using doctest::test_suite;

TEST_CASE("Position of the argument with default value") {
  argparse::ArgumentParser program("test");
  program.add_argument("-g").default_value("the_default_value");
  program.add_argument("-s");

  SUBCASE("Arg with default value not passed") {
    REQUIRE_NOTHROW(program.parse_args({"test", "-s", "./src"}));
    REQUIRE(program.get("-g") == std::string("the_default_value"));
    REQUIRE(program.get("-s") == std::string("./src"));
  }

  SUBCASE("Arg with default value passed last") {
    REQUIRE_NOTHROW(program.parse_args({"test", "-s", "./src", "-g"}));
    REQUIRE(program.get("-g") == std::string("the_default_value"));
    REQUIRE(program.get("-s") == std::string("./src"));
  }

  SUBCASE("Arg with default value passed before last") {
    REQUIRE_NOTHROW(program.parse_args({"test", "-g", "-s", "./src"}));
    REQUIRE(program.get("-g") == std::string("the_default_value"));
    REQUIRE(program.get("-s") == std::string("./src"));
  }

  SUBCASE("Arg with default value replaces the value if given") {
    REQUIRE_NOTHROW(program.parse_args({"test", "-g", "a_different_value", "-s", "./src"}));
    REQUIRE(program.get("-g") == std::string("a_different_value"));
    REQUIRE(program.get("-s") == std::string("./src"));
  }
}

TEST_CASE("Use a 'string' default value" * test_suite("default_value")) {
  argparse::ArgumentParser program("test");

  SUBCASE("Use a const char[] default value") {
    program.add_argument("--arg").default_value("array of char");
    REQUIRE_NOTHROW(program.parse_args({"test"}));
    REQUIRE(program.get("--arg") == std::string("array of char"));
  }

  SUBCASE("Use a std::string default value") {
    program.add_argument("--arg").default_value(std::string("string object"));
    REQUIRE_NOTHROW(program.parse_args({"test"}));
    REQUIRE(program.get("--arg") == std::string("string object"));
  }
}
