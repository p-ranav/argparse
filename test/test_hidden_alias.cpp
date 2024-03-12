#ifdef WITH_MODULE
import argparse;
#else
#include <argparse/argparse.hpp>
#endif
#include <doctest.hpp>

using doctest::test_suite;

TEST_CASE("Test setting a hidden alias for an argument" *
          test_suite("hidden_alias")) {
  argparse::ArgumentParser program("test");
  auto &arg = program.add_argument("--suppress").flag();
  program.add_hidden_alias_for(arg, "--supress"); // old misspelled alias

  program.parse_args({"./test.exe", "--supress"});
  REQUIRE(program.get<bool>("--suppress") == true);
}
