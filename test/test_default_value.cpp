#ifdef WITH_MODULE
import argparse;
#else
#include <argparse/argparse.hpp>
#endif
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

TEST_CASE("Use a default value with flag arguments" *
          test_suite("default_value")) {

  argparse::ArgumentParser program("test");

  program.add_argument("-inc_chr", "--include_chromes")
      .help(std::string{"only process the anchor whose one of the end is "
                        "contained on the specified "
                        "chromatin, used ',' to split."})
      .default_value("all");

  program.add_argument("-l").default_value(false).implicit_value(true);
  program.add_argument("-o").default_value(false).implicit_value(true);

  program.add_argument("filename");

  SUBCASE("Leading optional argument with default_value") {
    REQUIRE_NOTHROW(program.parse_args({"test", "-inc_chr", "-lo", "my.log"}));
    REQUIRE(program.get("-inc_chr") == std::string{"all"});
  }

  SUBCASE("Trailing optional argument with default_value") {
    REQUIRE_NOTHROW(program.parse_args({"test", "-lo", "my.log", "-inc_chr"}));
    REQUIRE(program.get("-inc_chr") == std::string{"all"});
  }
}

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
    REQUIRE_NOTHROW(
        program.parse_args({"test", "-g", "a_different_value", "-s", "./src"}));
    REQUIRE(program.get("-g") == std::string("a_different_value"));
    REQUIRE(program.get("-s") == std::string("./src"));
  }
}