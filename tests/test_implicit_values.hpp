#pragma once
#include <catch.hpp>
#include <argparse.hpp>

TEST_CASE("Parse toggle arguments with default value", "[parse_args]") {
  argparse::ArgumentParser program("test");
  program.add_argument("--verbose", "-v")
    .default_value(false)
    .implicit_value(true);

  program.parse_args({ "./test.exe" });
  auto arguments = program.get_arguments();
  REQUIRE(arguments.size() == 2);
  REQUIRE(program.get<bool>("--verbose") == false);
}

TEST_CASE("Parse toggle arguments with implicit value", "[parse_args]") {
  argparse::ArgumentParser program("test");
  program.add_argument("--verbose")
    .default_value(false)
    .implicit_value(true);

  program.parse_args({ "./test.exe", "--verbose" });
  auto arguments = program.get_arguments();
  REQUIRE(arguments.size() == 1);
  REQUIRE(program.get<bool>("--verbose") == true);
}