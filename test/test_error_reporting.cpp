#ifdef WITH_MODULE
import argparse;
#else
#include <argparse/argparse.hpp>
#endif
#include <doctest.hpp>

#include <iostream>
#include <string>
#include <vector>

using doctest::test_suite;

TEST_CASE("Missing optional argument name" * test_suite("error_reporting")) {
  argparse::ArgumentParser parser("test");
  parser.add_argument("-a");
  parser.add_argument("-b");

  SUBCASE("Good case") {
    REQUIRE_NOTHROW(parser.parse_args({"test", "-a", "1", "-b", "2"}));
  }

  SUBCASE("Bad case") {
    REQUIRE_THROWS_WITH_AS(
        parser.parse_args({"test", "-a", "1", "2"}),
        "Zero positional arguments expected, did you mean '[-b VAR]'",
        std::runtime_error);
  }

  SUBCASE("Bad case 2") {
    REQUIRE_THROWS_WITH_AS(
        parser.parse_args({"test", "1", "2"}),
        "Zero positional arguments expected, did you mean '[-a VAR]'",
        std::runtime_error);
  }
}

TEST_CASE("Missing optional argument name with other positional arguments" *
          test_suite("error_reporting")) {
  argparse::ArgumentParser parser("test");
  parser.add_argument("-a");
  parser.add_argument("-b");
  parser.add_argument("c");

  SUBCASE("Good case") {
    REQUIRE_NOTHROW(parser.parse_args({"test", "-a", "1", "-b", "2", "3"}));
  }

  SUBCASE("Bad case") {
    REQUIRE_THROWS_WITH_AS(
        parser.parse_args({"test", "-a", "1", "2", "3", "4"}),
        "Maximum number of positional arguments exceeded, failed to parse '3'",
        std::runtime_error);
  }
}