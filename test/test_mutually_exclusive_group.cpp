#ifdef WITH_MODULE
import argparse;
#else
#include <argparse/argparse.hpp>
#endif
#include <doctest.hpp>

using doctest::test_suite;

TEST_CASE("Create mutually exclusive group with 2 arguments" *
          test_suite("mutex_args")) {
  argparse::ArgumentParser program("test");

  auto &group = program.add_mutually_exclusive_group();
  group.add_argument("--first");
  group.add_argument("--second");

  REQUIRE_THROWS_WITH_AS(
      program.parse_args({"test", "--first", "1", "--second", "2"}),
      "Argument '--second VAR' not allowed with '--first VAR'",
      std::runtime_error);
}

TEST_CASE(
    "Create mutually exclusive group with 2 arguments, then copy the parser" *
    test_suite("mutex_args")) {
  argparse::ArgumentParser program("test");

  auto &group = program.add_mutually_exclusive_group();
  group.add_argument("--first");
  group.add_argument("--second");

  auto program_copy(program);

  REQUIRE_THROWS_WITH_AS(
      program_copy.parse_args({"test", "--first", "1", "--second", "2"}),
      "Argument '--second VAR' not allowed with '--first VAR'",
      std::runtime_error);
}

TEST_CASE("Create mutually exclusive group with 3 arguments" *
          test_suite("mutex_args")) {
  argparse::ArgumentParser program("test");

  auto &group = program.add_mutually_exclusive_group();
  group.add_argument("--first");
  group.add_argument("--second");
  group.add_argument("--third");

  REQUIRE_THROWS_WITH_AS(
      program.parse_args({"test", "--first", "1", "--third", "2"}),
      "Argument '--third VAR' not allowed with '--first VAR'",
      std::runtime_error);
}