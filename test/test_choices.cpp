#ifdef WITH_MODULE
import argparse;
#else
#include <argparse/argparse.hpp>
#endif

#include <doctest.hpp>

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
          "other positional argument" *
          test_suite("choices")) {
  argparse::ArgumentParser program("test");
  program.add_argument("--input")
      .default_value(std::string{"baz"})
      .choices("foo", "bar", "baz");
  program.add_argument("--value").scan<'i', int>().default_value(0);

  REQUIRE_NOTHROW(
      program.parse_args({"test", "--input", "foo", "--value", "1"}));
  REQUIRE(program.get("--input") == "foo");
  REQUIRE(program.get<int>("--value") == 1);
}

TEST_CASE(
    "Parse nargs argument that is in the fixed number of allowed choices, with "
    "other positional argument" *
    test_suite("choices")) {
  argparse::ArgumentParser program("test");
  program.add_argument("--input")
      .default_value(std::string{"baz"})
      .choices("foo", "bar", "baz")
      .nargs(2);
  program.add_argument("--value").scan<'i', int>().default_value(0);

  REQUIRE_NOTHROW(
      program.parse_args({"test", "--input", "foo", "bar", "--value", "1"}));
  REQUIRE((program.get<std::vector<std::string>>("--input") ==
           std::vector<std::string>{"foo", "bar"}));
  REQUIRE(program.get<int>("--value") == 1);
}

TEST_CASE("Parse argument that is in the fixed number of allowed choices, with "
          "other positional argument (reversed)" *
          test_suite("choices")) {
  argparse::ArgumentParser program("test");
  program.add_argument("--input")
      .default_value(std::string{"baz"})
      .choices("foo", "bar", "baz");
  program.add_argument("--value").scan<'i', int>().default_value(0);

  REQUIRE_NOTHROW(
      program.parse_args({"test", "--value", "1", "--input", "foo"}));
  REQUIRE(program.get("--input") == "foo");
  REQUIRE(program.get<int>("--value") == 1);
}

TEST_CASE(
    "Parse nargs argument that is in the fixed number of allowed choices, with "
    "other positional argument (reversed)" *
    test_suite("choices")) {
  argparse::ArgumentParser program("test");
  program.add_argument("--input")
      .default_value(std::string{"baz"})
      .choices("foo", "bar", "baz")
      .nargs(2);
  program.add_argument("--value").scan<'i', int>().default_value(0);

  REQUIRE_NOTHROW(
      program.parse_args({"test", "--value", "1", "--input", "foo", "bar"}));
  REQUIRE((program.get<std::vector<std::string>>("--input") ==
           std::vector<std::string>{"foo", "bar"}));
  REQUIRE(program.get<int>("--value") == 1);
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

TEST_CASE("Show allowed choices in help test") {
  argparse::ArgumentParser program("test");
  program.add_argument("--color").choices("red", "green", "blue");
  program.add_argument("--index").choices(1, 2, 3);

  auto help_output = program.help().str();

  REQUIRE(help_output.find(std::string{"[allowed: red, green, blue]"}) !=
          std::string::npos);
  REQUIRE(help_output.find(std::string{"[allowed: 1, 2, 3]"}) !=
          std::string::npos);
}
