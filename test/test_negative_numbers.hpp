#pragma once
#include <catch.hpp>
#include <argparse.hpp>

TEST_CASE("Parse negative integer", "[positional_arguments]") {
  argparse::ArgumentParser program;
  program.add_argument("--verbose", "-v")
    .help("enable verbose logging")
    .default_value(false)
    .implicit_value(true);

  program.add_argument("number")
    .help("Input number")
    .action([](const std::string& value) { return std::stoi(value); });

  program.parse_args({"./main", "-1"});
  REQUIRE(program.get<int>("number") == -1);
}

TEST_CASE("Parse negative integers into a vector", "[positional_arguments]") {
  argparse::ArgumentParser program;
  program.add_argument("--verbose", "-v")
    .help("enable verbose logging")
    .default_value(false)
    .implicit_value(true);

  program.add_argument("number")
    .help("Input number")
    .nargs(3)
    .action([](const std::string& value) { return std::stoi(value); });

  program.parse_args({"./main", "-1", "-2", "3"});
  REQUIRE(program["number"] == std::vector<int>{-1, -2, 3});
}

TEST_CASE("Parse negative float", "[positional_arguments]") {
  argparse::ArgumentParser program;
  program.add_argument("--verbose", "-v")
    .help("enable verbose logging")
    .default_value(false)
    .implicit_value(true);

  program.add_argument("number")
    .help("Input number")
    .action([](const std::string& value) { return std::stof(value); });

  program.parse_args({"./main", "-1.0"});
  REQUIRE(program.get<float>("number") == -1.0);
}

TEST_CASE("Parse negative floats into a vector", "[positional_arguments]") {
  argparse::ArgumentParser program;
  program.add_argument("--verbose", "-v")
    .help("enable verbose logging")
    .default_value(false)
    .implicit_value(true);

  program.add_argument("number")
    .help("Input number")
    .nargs(3)
    .action([](const std::string& value) { return std::stod(value); });

  program.parse_args({"./main", "-1.001", "-2.002", "3.003"});
  REQUIRE(program["number"] == std::vector<double>{-1.001, -2.002, 3.003});
}

TEST_CASE("Parse numbers in E notation", "[positional_arguments]") {
  argparse::ArgumentParser program;
  program.add_argument("--verbose", "-v")
    .help("enable verbose logging")
    .default_value(false)
    .implicit_value(true);

  program.add_argument("number")
    .help("Input number")
    .action([](const std::string& value) { return std::stod(value); });

  program.parse_args({"./main", "-1.2e3"});
  REQUIRE(program.get<double>("number") == -1200.0);
}

TEST_CASE("Parse numbers in E notation (capital E)", "[positional_arguments]") {
  argparse::ArgumentParser program;
  program.add_argument("--verbose", "-v")
    .help("enable verbose logging")
    .default_value(false)
    .implicit_value(true);

  program.add_argument("number")
    .help("Input number")
    .action([](const std::string& value) { return std::stod(value); });

  program.parse_args({"./main", "-1.32E4"});
  REQUIRE(program.get<double>("number") == -13200.0);
}
