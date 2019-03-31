#pragma once
#include <catch.hpp>
#include <argparse.hpp>

TEST_CASE("Parse toggle arguments with default value", "[optional_arguments]") {
  argparse::ArgumentParser program("test");
  program.add_argument("--verbose", "-v")
    .default_value(false)
    .implicit_value(true);

  program.parse_args({ "./test.exe" });
  auto arguments = program.get_arguments();
  REQUIRE(arguments.size() == 2);
  REQUIRE(program.get<bool>("--verbose") == false);
  REQUIRE(program["--verbose"] == false);
}

TEST_CASE("Parse toggle arguments with implicit value", "[optional_arguments]") {
  argparse::ArgumentParser program("test");
  program.add_argument("--verbose")
    .default_value(false)
    .implicit_value(true);

  program.parse_args({ "./test.exe", "--verbose" });
  auto arguments = program.get_arguments();
  REQUIRE(arguments.size() == 1);
  REQUIRE(program.get<bool>("--verbose") == true);
  REQUIRE(program["--verbose"] == true);
  REQUIRE(program["--verbose"] != false);
}

TEST_CASE("Parse multiple toggle arguments with implicit values", "[optional_arguments]") {
  argparse::ArgumentParser program("test");
  program.add_argument("-a")
    .default_value(false)
    .implicit_value(true);

  program.add_argument("-u")
    .default_value(false)
    .implicit_value(true);

  program.add_argument("-x")
    .default_value(false)
    .implicit_value(true);

  program.parse_args({ "./test.exe", "-a", "-x" });
  auto arguments = program.get_arguments();
  REQUIRE(arguments.size() == 3);
  REQUIRE(program.get<bool>("-a") == true);
  REQUIRE(program.get<bool>("-u") == false);
  REQUIRE(program.get<bool>("-x") == true);
}