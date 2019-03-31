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

TEST_CASE("Parse multiple toggle arguments with implicit values", "[parse_args]") {
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

TEST_CASE("Parse compound toggle arguments with implicit values", "[parse_args]") {
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

  program.parse_args({ "./test.exe", "-aux" });
  auto arguments = program.get_arguments();
  REQUIRE(arguments.size() == 3);
  REQUIRE(program.get<bool>("-a") == true);
  REQUIRE(program.get<bool>("-u") == true);
  REQUIRE(program.get<bool>("-x") == true);
}

TEST_CASE("Parse compound toggle arguments with implicit values and nargs", "[parse_args]") {
  argparse::ArgumentParser program("test");
  program.add_argument("-a")
    .default_value(false)
    .implicit_value(true);

  program.add_argument("-b")
    .default_value(false)
    .implicit_value(true);

  program.add_argument("-c")
    .nargs(2)
    .action([](const std::string& value) { return std::stof(value); });

  program.parse_args({ "./test.exe", "-abc", "3.14", "2.718" });
  auto arguments = program.get_arguments();
  REQUIRE(arguments.size() == 3);
  REQUIRE(program.get<bool>("-a") == true);
  REQUIRE(program.get<bool>("-b") == true);
  auto c = program.get<std::vector<float>>("-c");
  REQUIRE(c.size() == 2);
  REQUIRE(c[0] == 3.14f);
  REQUIRE(c[1] == 2.718f);
}