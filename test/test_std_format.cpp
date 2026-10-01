#ifdef WITH_MODULE
import argparse;
#else
#include <argparse/argparse.hpp>
#endif
#include <doctest.hpp>

// Only compiled when the standard library provides std::formatter (C++20).
#ifdef ARGPARSE_HAS_STD_FORMAT

#include <format>

using doctest::test_suite;

TEST_CASE("std::format an ArgumentParser" * test_suite("std_format")) {
  argparse::ArgumentParser program("test");
  program.add_argument("--verbose").flag();
  program.add_argument("input");
  program.parse_args({"test", "--verbose", "file.txt"});

  std::ostringstream expected;
  expected << program;
  REQUIRE(std::format("{}", program) == expected.str());
}

TEST_CASE("std::format an Argument" * test_suite("std_format")) {
  argparse::ArgumentParser program("test");
  auto &argument = program.add_argument("--verbose").help("be verbose");

  std::ostringstream expected;
  expected << argument;
  REQUIRE(std::format("{}", argument) == expected.str());
}

#endif // ARGPARSE_HAS_STD_FORMAT
