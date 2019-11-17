#pragma once
#include <catch.hpp>
#include <argparse.hpp>

TEST_CASE("Parse toggle arguments with default value", "[optional_arguments]") {
  argparse::ArgumentParser program("test");
  program.add_argument("--verbose", "-v")
    .default_value(false)
    .implicit_value(true);

  program.parse_args({ "./test.exe" });
  REQUIRE(program.get<bool>("--verbose") == false);
  REQUIRE(program["--verbose"] == false);
}

TEST_CASE("Parse toggle arguments with implicit value", "[optional_arguments]") {
  argparse::ArgumentParser program("test");
  program.add_argument("--verbose")
    .default_value(false)
    .implicit_value(true);

  program.parse_args({ "./test.exe", "--verbose" });
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
  REQUIRE(program.get<bool>("-a") == true);
  REQUIRE(program.get<bool>("-u") == false);
  REQUIRE(program.get<bool>("-x") == true);
}

TEST_CASE("Parse arguments of different types", "[optional_arguments]") {
  using namespace std::literals;

  argparse::ArgumentParser program("test");
  program.add_argument("--this-argument-is-longer-than-any-sso-buffer-that-"
                       "makes-sense-unless-your-cache-line-is-this-long"s);

  REQUIRE_NOTHROW(program.parse_args({"test"}));

  program.add_argument("-string"s, "-string-view"sv, "-builtin")
      .default_value(false)
      .implicit_value(true);

  program.parse_args({"test", "-string-view"});
  REQUIRE(program["-string"sv] == true);
  REQUIRE(program["-string-view"] == true);
  REQUIRE(program["-builtin"s] == true);
}
