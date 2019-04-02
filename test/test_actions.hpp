#pragma once
#include <catch.hpp>
#include <argparse.hpp>

TEST_CASE("Users can use defaul value inside actions", "[actions]") {
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