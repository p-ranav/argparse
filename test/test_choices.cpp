#ifdef WITH_MODULE
import argparse;
#else
#include <argparse/argparse.hpp>
#endif

#include "doctest.hpp"

using doctest::test_suite;

TEST_CASE("Parse argument that is provided zero choices" *
          test_suite("choices")) {
  argparse::ArgumentParser program("test");
  REQUIRE_THROWS_WITH_AS(program.add_argument("color").choices(),
                         "Zero choices provided", std::runtime_error);
}

TEST_CASE("Parse argument that is in the fixed number of allowed choices" *
          test_suite("choices")) {
  argparse::ArgumentParser program("test");
  program.add_argument("color").choices("red", "green", "blue");

  program.parse_args({"test", "red"});
}

TEST_CASE("Parse argument that is in the fixed number of allowed choices, with "
          "invalid default" *
          test_suite("choices")) {
  argparse::ArgumentParser program("test");
  program.add_argument("color").default_value("yellow").choices("red", "green",
                                                                "blue");

  REQUIRE_THROWS_WITH_AS(
      program.parse_args({"test"}),
      "Invalid default value \"yellow\" - allowed options: {red, green, blue}",
      std::runtime_error);
}

TEST_CASE("Parse invalid argument that is not in the fixed number of allowed "
          "choices" *
          test_suite("choices")) {
  argparse::ArgumentParser program("test");
  program.add_argument("color").choices("red", "green", "blue");

  REQUIRE_THROWS_WITH_AS(
      program.parse_args({"test", "red2"}),
      "Invalid argument \"red2\" - allowed options: {red, green, blue}",
      std::runtime_error);
}

TEST_CASE(
    "Parse multiple arguments that are in the fixed number of allowed choices" *
    test_suite("choices")) {
  argparse::ArgumentParser program("test");
  program.add_argument("color").nargs(2).choices("red", "green", "blue");

  program.parse_args({"test", "red", "green"});
}

TEST_CASE("Parse multiple arguments one of which is not in the fixed number of "
          "allowed choices" *
          test_suite("choices")) {
  argparse::ArgumentParser program("test");
  program.add_argument("color").nargs(2).choices("red", "green", "blue");

  REQUIRE_THROWS_WITH_AS(
      program.parse_args({"test", "red", "green2"}),
      "Invalid argument \"green2\" - allowed options: {red, green, blue}",
      std::runtime_error);
}

TEST_CASE("Parse multiple arguments that are in the fixed number of allowed "
          "INTEGER choices" *
          test_suite("choices")) {
  argparse::ArgumentParser program("test");
  program.add_argument("indices").nargs(2).choices(1, 2, 3, 4, 5);

  program.parse_args({"test", "1", "2"});
}

TEST_CASE("Parse multiple arguments that are not in fixed number of allowed "
          "INTEGER choices" *
          test_suite("choices")) {
  argparse::ArgumentParser program("test");
  program.add_argument("indices").nargs(2).choices(1, 2, 3, 4, 5);

  REQUIRE_THROWS_WITH_AS(
      program.parse_args({"test", "6", "7"}),
      "Invalid argument \"6\" - allowed options: {1, 2, 3, 4, 5}",
      std::runtime_error);
}