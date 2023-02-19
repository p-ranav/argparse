#include <argparse/argparse.hpp>
#include <doctest.hpp>
#include <string>

using doctest::test_suite;

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
