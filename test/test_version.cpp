#include <doctest.hpp>
#include <argparse/argparse.hpp>

using doctest::test_suite;

TEST_CASE("Users can print version  and exit" * test_suite("version")) {
  argparse::ArgumentParser program("cli-test", "1.9.0");
  program.add_argument("-d", "--dir")
    .required();
  program.parse_args( { "test", "--version" });
  REQUIRE(program.get("--version") == "1.9.0");
}
