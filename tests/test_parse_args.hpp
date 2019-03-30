#pragma once
#include <catch.hpp>
#include <argparse.hpp>

TEST_CASE("Parse a string argument with value", "[parse_args]") {
  argparse::ArgumentParser program("test");
  program.add_argument("--config");
  program.parse_args({ "test", "--config", "config.yml"});
  auto arguments = program.get_arguments();
  REQUIRE(arguments.size() == 1);
  REQUIRE(program.get("--config") == "config.yml");
}

TEST_CASE("Parse a string argument with default value", "[parse_args]") {
  argparse::ArgumentParser program("test");
  program.add_argument("--config")
    .default_value([]() { return std::string("foo.yml"); });
  program.parse_args({ "test", "--config" });
  auto arguments = program.get_arguments();
  REQUIRE(arguments.size() == 1);
  REQUIRE(program.get("--config") == "foo.yml");
}

TEST_CASE("Parse an int argument with value", "[parse_args]") {
  argparse::ArgumentParser program("test");
  program.add_argument("--count")
    .action([](const std::string& value) { return std::stoi(value); });
  program.parse_args({ "test", "--count", "5" });
  auto arguments = program.get_arguments();
  REQUIRE(arguments.size() == 1);
  REQUIRE(program.get<int>("--count") == 5);
}

TEST_CASE("Parse an int argument with default value", "[parse_args]") {
  argparse::ArgumentParser program("test");
  program.add_argument("--count")
    .default_value([]() { return 2; })
    .action([](const std::string& value) { return std::stoi(value); });
  program.parse_args({ "test", "--count" });
  auto arguments = program.get_arguments();
  REQUIRE(arguments.size() == 1);
  REQUIRE(program.get<int>("--count") == 2);
}

TEST_CASE("Parse a float argument with value", "[parse_args]") {
  argparse::ArgumentParser program("test");
  program.add_argument("--ratio")
    .action([](const std::string& value) { return std::stof(value); });
  program.parse_args({ "test", "--ratio", "5.6645" });
  auto arguments = program.get_arguments();
  REQUIRE(arguments.size() == 1);
  REQUIRE(program.get<float>("--ratio") == 5.6645f);
}

TEST_CASE("Parse a float argument with default value", "[parse_args]") {
  argparse::ArgumentParser program("test");
  program.add_argument("--ratio")
    .default_value([]() { return 3.14f; })
    .action([](const std::string& value) { return std::stof(value); });
  program.parse_args({ "test", "--ratio" });
  auto arguments = program.get_arguments();
  REQUIRE(arguments.size() == 1);
  REQUIRE(program.get<float>("--ratio") == 3.14f);
}

TEST_CASE("Parse a double argument with value", "[parse_args]") {
  argparse::ArgumentParser program("test");
  program.add_argument("--ratio")
    .action([](const std::string& value) { return std::stod(value); });
  program.parse_args({ "test", "--ratio", "5.6645" });
  auto arguments = program.get_arguments();
  REQUIRE(arguments.size() == 1);
  REQUIRE(program.get<double>("--ratio") == 5.6645);
}

TEST_CASE("Parse a double argument with default value", "[parse_args]") {
  argparse::ArgumentParser program("test");
  program.add_argument("--ratio")
    .default_value([]() { return 3.14; })
    .action([](const std::string& value) { return std::stod(value); });
  program.parse_args({ "test", "--ratio" });
  auto arguments = program.get_arguments();
  REQUIRE(arguments.size() == 1);
  REQUIRE(program.get<double>("--ratio") == 3.14);
}