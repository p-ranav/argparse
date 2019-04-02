#pragma once
#include <catch.hpp>
#include <argparse.hpp>

TEST_CASE("Add parent parsers", "[parent_parsers]") {
  argparse::ArgumentParser parent_parser("main");
  parent_parser.add_argument("--verbose")
    .default_value(false)
    .implicit_value(true);

  argparse::ArgumentParser child_parser("foo");
  child_parser.add_parents(parent_parser);
  child_parser.parse_args({ "./main", "--verbose"});
  REQUIRE(child_parser["--verbose"] == true);
}

TEST_CASE("Add parent to multiple parent parsers", "[parent_parsers]") {
  argparse::ArgumentParser parent_parser("main");
  parent_parser.add_argument("--parent")
    .default_value(0)
    .action([](const std::string& value) { return std::stoi(value); });

  argparse::ArgumentParser foo_parser("foo");
  foo_parser.add_argument("foo");
  foo_parser.add_parents(parent_parser);
  foo_parser.parse_args({ "./main", "--parent", "2", "XXX" });
  REQUIRE(foo_parser["--parent"] == 2);
  REQUIRE(foo_parser["foo"] == std::string("XXX"));

  argparse::ArgumentParser bar_parser("bar");
  bar_parser.add_argument("--bar");
  bar_parser.parse_args({ "./main", "--bar", "YYY" });
  REQUIRE(bar_parser["--bar"] == std::string("YYY"));
}