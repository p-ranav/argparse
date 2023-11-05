#ifdef WITH_MODULE
import argparse;
#else
#include <argparse/argparse.hpp>
#endif
#include <doctest.hpp>
#include <test_utility.hpp>

using doctest::test_suite;

TEST_CASE("Parse positional arguments using a copy of an ArgumentParser" * test_suite("vector")) {

  auto maker = []() {
	argparse::ArgumentParser program("test");
	program.add_argument("first");
	program.add_argument("second");

	return program;
  };

  auto program = maker();

  program.parse_args({"test", "rocket.mesh", "thrust_profile.csv"});
}