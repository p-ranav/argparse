#ifdef WITH_MODULE
import argparse;
#else
#include <argparse/argparse.hpp>
#endif
#include <doctest.hpp>

using doctest::test_suite;

TEST_CASE("User-supplied argument" * test_suite("is_used")) {
  argparse::ArgumentParser program("test");
  program.add_argument("--dir").default_value(std::string("/"));
  program.parse_args({"test", "--dir", "/home/user"});
  REQUIRE(program.get("--dir") == "/home/user");
  REQUIRE(program.is_used("--dir") == true);
}

TEST_CASE("Not user-supplied argument" * test_suite("is_used")) {
  argparse::ArgumentParser program("test");
  program.add_argument("--dir").default_value(std::string("/"));
  program.parse_args({"test"});
  REQUIRE(program.get("--dir") == "/");
  REQUIRE(program.is_used("--dir") == false);
}

TEST_CASE("Argument::is_used avoids repeating the argument name" *
          test_suite("is_used")) {
  argparse::ArgumentParser program("test");
  auto &dir_arg = program.add_argument("--dir").default_value(std::string("/"));
  program.parse_args({"test", "--dir", "/home/user"});
  REQUIRE(dir_arg.is_used() == true);
}

TEST_CASE("Argument::is_used is false when not supplied" *
          test_suite("is_used")) {
  argparse::ArgumentParser program("test");
  auto &dir_arg = program.add_argument("--dir").default_value(std::string("/"));
  program.parse_args({"test"});
  REQUIRE(dir_arg.is_used() == false);
}

TEST_CASE("Argument::operator bool reflects usage" * test_suite("is_used")) {
  argparse::ArgumentParser program("test");
  auto &flag_arg = program.add_argument("--flag").flag();
  program.parse_args({"test", "--flag"});
  REQUIRE(static_cast<bool>(flag_arg));
  REQUIRE(flag_arg);
}

TEST_CASE("Argument::operator bool is false when not used" *
          test_suite("is_used")) {
  argparse::ArgumentParser program("test");
  auto &flag_arg = program.add_argument("--flag").flag();
  program.parse_args({"test"});
  REQUIRE_FALSE(static_cast<bool>(flag_arg));
}
