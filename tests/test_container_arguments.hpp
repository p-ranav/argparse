#pragma once
#include <catch.hpp>
#include <argparse.hpp>

TEST_CASE("Parse vector of arguments", "[vector]") {
  argparse::ArgumentParser program("test");
  program.add_argument("input")
    .nargs(2);

  program.parse_args({ "test", "rocket.mesh", "thrust_profile.csv" });

  auto arguments = program.get_arguments();
  REQUIRE(arguments.size() == 1);

  auto inputs = program.get<std::vector<std::string>>("input");
  REQUIRE(inputs.size() == 2);
  REQUIRE(inputs[0] == "rocket.mesh");
  REQUIRE(inputs[1] == "thrust_profile.csv");
}

TEST_CASE("Parse list of arguments", "[vector]") {
  argparse::ArgumentParser program("test");
  program.add_argument("input")
    .nargs(2);

  program.parse_args({ "test", "rocket.mesh", "thrust_profile.csv" });

  auto arguments = program.get_arguments();
  REQUIRE(arguments.size() == 1);
  auto inputs = program.get<std::list<std::string>>("input");
  REQUIRE(inputs.size() == 2);
  REQUIRE(argparse::get_from_list(inputs, 0) == "rocket.mesh");
  REQUIRE(argparse::get_from_list(inputs, 1) == "thrust_profile.csv");
}

TEST_CASE("Parse list of arguments with default values", "[vector]") {
  argparse::ArgumentParser program("test");
  program.add_argument("input")
    .default_value(std::list<int>{1, 2, 3, 4, 5})
    .nargs(2);

  program.parse_args({ "test" });

  auto arguments = program.get_arguments();
  REQUIRE(arguments.size() == 1);
  auto inputs = program.get<std::list<int>>("input");
  REQUIRE(inputs.size() == 5);
  REQUIRE(argparse::get_from_list(inputs, 0) == 1);
  REQUIRE(argparse::get_from_list(inputs, 1) == 2);
  REQUIRE(argparse::get_from_list(inputs, 2) == 3);
  REQUIRE(argparse::get_from_list(inputs, 3) == 4);
  REQUIRE(argparse::get_from_list(inputs, 4) == 5);
}