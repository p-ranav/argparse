#include <argparse/argparse.hpp>
#include <doctest.hpp>
#include <sstream>
#include <streambuf>

using doctest::test_suite;

TEST_CASE("Include all default arguments" * test_suite("default_args")) {
  argparse::ArgumentParser parser("test");
  auto help_msg{parser.help().str()};
  REQUIRE(help_msg.find("shows help message") != std::string::npos);
  REQUIRE(help_msg.find("prints version information") != std::string::npos);
}

TEST_CASE("Do not include default arguments" * test_suite("default_args")) {
  argparse::ArgumentParser parser("test", "1.0",
                                  argparse::default_arguments::none);
  parser.parse_args({"test"});
  REQUIRE_THROWS_AS(parser.get("--help"), std::logic_error);
  REQUIRE_THROWS_AS(parser.get("--version"), std::logic_error);
}

TEST_CASE("Do not exit on default arguments" * test_suite("default_args")) {
  argparse::ArgumentParser parser("test", "1.0",
                                  argparse::default_arguments::all, false);
  std::stringstream buf;
  std::streambuf* saved_cout_buf = std::cout.rdbuf(buf.rdbuf());
  parser.parse_args({"test", "--help"});
  std::cout.rdbuf(saved_cout_buf);
  REQUIRE(parser.is_used("--help"));
}
