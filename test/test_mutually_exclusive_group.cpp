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
    "Create mutually exclusive group with 2 arguments with required flag" *
    test_suite("mutex_args")) {
  argparse::ArgumentParser program("test");

  auto &group = program.add_mutually_exclusive_group(true);
  group.add_argument("--first");
  group.add_argument("--second");

  REQUIRE_THROWS_WITH_AS(
      program.parse_args({"test"}),
      "One of the arguments '--first VAR' or '--second VAR' is required",
      std::runtime_error);
}

TEST_CASE(
    "Create mutually exclusive group with 3 arguments with required flag" *
    test_suite("mutex_args")) {
  argparse::ArgumentParser program("test");

  auto &group = program.add_mutually_exclusive_group(true);
  group.add_argument("--first");
  group.add_argument("--second");
  group.add_argument("--third");

  REQUIRE_THROWS_WITH_AS(program.parse_args({"test"}),
                         "One of the arguments '--first VAR' or '--second VAR' "
                         "or '--third VAR' is required",
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

TEST_CASE("Create two mutually exclusive groups" * test_suite("mutex_args")) {
  argparse::ArgumentParser program("test");

  auto &group_1 = program.add_mutually_exclusive_group();
  group_1.add_argument("--first");
  group_1.add_argument("--second");
  group_1.add_argument("--third");

  auto &group_2 = program.add_mutually_exclusive_group();
  group_2.add_argument("-a");
  group_2.add_argument("-b");

  REQUIRE_THROWS_WITH_AS(
      program.parse_args({"test", "--first", "1", "-a", "2", "-b", "3"}),
      "Argument '-b VAR' not allowed with '-a VAR'", std::runtime_error);
}