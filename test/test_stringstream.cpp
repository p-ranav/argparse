#ifdef WITH_MODULE
import argparse;
#else
#include <argparse/argparse.hpp>
#endif
#include "doctest.hpp"

#include <cmath>
#include <string>
#include <vector>

using doctest::test_suite;

TEST_CASE("Get Version String" * test_suite("stringstream")) {
  std::stringstream os;
  argparse::ArgumentParser program("test", "1.0",
                                   argparse::default_arguments::all, false, os);
  program.parse_args({"test", "--version"});
  REQUIRE(os.str() == "1.0\n");
}