#pragma once
#include <catch.hpp>
#include <argparse.hpp>

TEST_CASE("Parse positional arguments", "[positional_arguments]") {
  argparse::ArgumentParser program("test");
  program.add_argument("input");
  program.add_argument("output");
  program.parse_args({ "test", "rocket.mesh", "thrust_profile.csv" });
  auto arguments = program.get_arguments();
  REQUIRE(arguments.size() == 2);
  REQUIRE(program.get("input") == "rocket.mesh");
  REQUIRE(program.get("output") == "thrust_profile.csv");
}

TEST_CASE("Parse positional arguments with fixed nargs", "[positional_arguments]") {
  argparse::ArgumentParser program("test");
  program.add_argument("input");
  program.add_argument("output").nargs(2);
  program.parse_args({ "test", "rocket.mesh", "thrust_profile.csv", "output.mesh" });
  auto arguments = program.get_arguments();
  REQUIRE(arguments.size() == 2);
  REQUIRE(program.get("input") == "rocket.mesh");
  auto outputs = program.get<std::vector<std::string>>("output");
  REQUIRE(outputs.size() == 2);
  REQUIRE(outputs[0] == "thrust_profile.csv");
  REQUIRE(outputs[1] == "output.mesh");
}

TEST_CASE("Parse positional arguments with optional arguments", "[positional_arguments]") {
  argparse::ArgumentParser program("test");
  program.add_argument("input");
  program.add_argument("output").nargs(2);
  program.add_argument("--num_iterations")
    .action([](const std::string& value) { return std::stoi(value); });
  program.parse_args({ "test", "rocket.mesh", "--num_iterations", "15", "thrust_profile.csv", "output.mesh" });
  auto arguments = program.get_arguments();
  REQUIRE(arguments.size() == 3);
  REQUIRE(program.get<int>("--num_iterations") == 15);
  REQUIRE(program.get("input") == "rocket.mesh");
  auto outputs = program.get<std::vector<std::string>>("output");
  REQUIRE(outputs.size() == 2);
  REQUIRE(outputs[0] == "thrust_profile.csv");
  REQUIRE(outputs[1] == "output.mesh");
}

TEST_CASE("Parse positional arguments with optional arguments in the middle", "[positional_arguments]") {
  argparse::ArgumentParser program("test");
  program.add_argument("input");
  program.add_argument("output").nargs(2);
  program.add_argument("--num_iterations")
    .action([](const std::string& value) { return std::stoi(value); });
  program.parse_args({ "test", "rocket.mesh", "thrust_profile.csv", "--num_iterations", "15", "output.mesh" });
  auto arguments = program.get_arguments();
  REQUIRE(arguments.size() == 3);
  REQUIRE(program.get<int>("--num_iterations") == 15);
  REQUIRE(program.get("input") == "rocket.mesh");
  auto outputs = program.get<std::vector<std::string>>("output");
  REQUIRE(outputs.size() == 2);
  REQUIRE(outputs[0] == "thrust_profile.csv");
  REQUIRE(outputs[1] == "output.mesh");
}

TEST_CASE("Square a number", "[positional_arguments]") {
  argparse::ArgumentParser program;
  program.add_argument("square")
    .help("display a square of a given number")
    .action([](const std::string& value) { auto integer = std::stoi(value); return integer * integer; });

  program.parse_args({"./main", "15"});
  REQUIRE(program.get<int>("square") == 225);
}