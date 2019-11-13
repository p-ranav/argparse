#pragma once
#include <catch.hpp>
#include <argparse.hpp>

TEST_CASE("Users can use default value inside actions", "[actions]") {
  argparse::ArgumentParser program("test");
  program.add_argument("input")
    .default_value("bar")
    .action([=](const std::string& value) {
      static const std::vector<std::string> choices = { "foo", "bar", "baz" };
      if (std::find(choices.begin(), choices.end(), value) != choices.end()) {
        return value;
      }
      return std::string{ "bar" };
    });

  program.parse_args({ "test", "fez" });
  REQUIRE(program.get("input") == "bar");
}

TEST_CASE("Users can add actions that return nothing", "[actions]") {
  argparse::ArgumentParser program("test");
  bool pressed = false;
  auto &arg = program.add_argument("button").action(
      [&](const std::string &) mutable { pressed = true; });

  REQUIRE_FALSE(pressed);

  SECTION("action performed") {
    program.parse_args({"test", "ignored"});
    REQUIRE(pressed);
  }

  SECTION("action performed and nothing overrides the default value") {
    arg.default_value(42);

    program.parse_args({"test", "ignored"});
    REQUIRE(pressed);
    REQUIRE(program.get<int>("button") == 42);
  }
}
