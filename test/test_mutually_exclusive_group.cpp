#ifdef WITH_MODULE
import argparse;
#else
#include <argparse/argparse.hpp>
#endif
#include <doctest.hpp>

using doctest::test_suite;

TEST_CASE("User-supplied argument" * test_suite("is_used")) {
  argparse::ArgumentParser program("test");

  auto &group = program.add_mutually_exclusive_group();
  group.add_argument("--first");
  group.add_argument("--second");

  REQUIRE_THROWS_WITH_AS(
      program.parse_args({"test", "--first", "1", "--second", "2"}),
      "Argument '--first VAR' not allowed with '--second VAR'",
      std::runtime_error);
}