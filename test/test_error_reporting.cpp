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
        "Zero positional arguments expected, did you mean '-b VAR'?",
        std::runtime_error);
  }

  SUBCASE("Bad case 2") {
    REQUIRE_THROWS_WITH_AS(
        parser.parse_args({"test", "1", "2"}),
        "Zero positional arguments expected, did you mean '-a VAR'?",
        std::runtime_error);
  }
}

TEST_CASE("Missing optional argument name (some flag arguments)" *
          test_suite("error_reporting")) {
  argparse::ArgumentParser parser("test");
  parser.add_argument("-a").flag();
  parser.add_argument("-b").flag();
  parser.add_argument("-c");
  parser.add_argument("-d");

  SUBCASE("Good case") {
    REQUIRE_NOTHROW(parser.parse_args({"test", "-a", "-b", "-c", "2"}));
  }

  SUBCASE("Bad case") {
    REQUIRE_THROWS_WITH_AS(
        parser.parse_args({"test", "-a", "-b", "2"}),
        "Zero positional arguments expected, did you mean '-c VAR'?",
        std::runtime_error);
  }

  SUBCASE("Bad case 2") {
    REQUIRE_THROWS_WITH_AS(
        parser.parse_args({"test", "-abc", "1", "2"}),
        "Zero positional arguments expected, did you mean '-d VAR'?",
        std::runtime_error);
  }
}

TEST_CASE("Missing optional argument name (multiple names)" *
          test_suite("error_reporting")) {
  argparse::ArgumentParser parser("test");
  parser.add_argument("-a", "--number-of-apples");
  parser.add_argument("-b");

  SUBCASE("Bad case 2") {
    REQUIRE_THROWS_WITH_AS(parser.parse_args({"test", "1", "2"}),
                           "Zero positional arguments expected, did you mean "
                           "'-a/--number-of-apples VAR'?",
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

TEST_CASE("Detect unknown subcommand" * test_suite("error_reporting")) {
  argparse::ArgumentParser program("git");
  argparse::ArgumentParser log_command("log");
  argparse::ArgumentParser notes_command("notes");
  argparse::ArgumentParser add_command("add");
  program.add_subparser(log_command);
  program.add_subparser(notes_command);
  program.add_subparser(add_command);

  SUBCASE("Typo for 'notes'") {
    REQUIRE_THROWS_WITH_AS(program.parse_args({"git", "tote"}),
                           "Failed to parse 'tote', did you mean 'notes'?",
                           std::runtime_error);
  }

  SUBCASE("Typo for 'add'") {
    REQUIRE_THROWS_WITH_AS(program.parse_args({"git", "bad"}),
                           "Failed to parse 'bad', did you mean 'add'?",
                           std::runtime_error);
  }

  SUBCASE("Typo for 'log'") {
    REQUIRE_THROWS_WITH_AS(program.parse_args({"git", "logic"}),
                           "Failed to parse 'logic', did you mean 'log'?",
                           std::runtime_error);
  }
}