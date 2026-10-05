#ifdef WITH_MODULE
import argparse;
#else
#include <argparse/argparse.hpp>
#endif
#include <cmath>
#include <doctest.hpp>

using doctest::test_suite;

TEST_CASE("Parse with custom prefix chars" * test_suite("prefix_chars")) {
  argparse::ArgumentParser program("test");
  program.set_prefix_chars("-+");
  program.add_argument("+f");
  program.add_argument("++bar");
  program.parse_args({"test", "+f", "X", "++bar", "Y"});
  REQUIRE(program.get("+f") == "X");
  REQUIRE(program.get("++bar") == "Y");
}

TEST_CASE("Parse with custom Windows-style prefix chars" *
          test_suite("prefix_chars")) {
  argparse::ArgumentParser program("dir");
  program.set_prefix_chars("/");
  program.add_argument("/A").nargs(1);
  program.add_argument("/B").flag();
  program.add_argument("/C").flag();
  program.parse_args({"dir", "/A", "D", "/B", "/C"});
  REQUIRE(program.get("/A") == "D");
  REQUIRE(program.get<bool>("/B") == true);
}

TEST_CASE("Parse with custom Windows-style prefix chars and assign chars" *
          test_suite("prefix_chars")) {
  argparse::ArgumentParser program("dir");
  program.set_prefix_chars("/");
  program.set_assign_chars(":=");
  program.add_argument("/A").nargs(1);
  program.add_argument("/B").nargs(1);
  program.add_argument("/C").flag();
  program.parse_args({"dir", "/A:D", "/B=Boo", "/C"});
  REQUIRE(program.get("/A") == "D");
  REQUIRE(program.get("/B") == "Boo");
  REQUIRE(program.get<bool>("/C") == true);
}

TEST_CASE("Compound arguments preserve their prefix" *
          test_suite("prefix_chars")) {
  for (const char prefix : {'-', '+', '/'}) {
    for (const bool known : {false, true}) {
      CAPTURE(prefix);
      CAPTURE(known);
      argparse::ArgumentParser program("test", "1.0",
                                       argparse::default_arguments::none);
      program.set_prefix_chars(std::string(1, prefix));
      const auto first = std::string{prefix, 'a'};
      const auto second = std::string{prefix, 'b'};
      program.add_argument(first).flag();
      program.add_argument(second);
      const std::vector<std::string> args{"test", std::string{prefix, 'a', 'b'},
                                          "value"};
      if (known) {
        REQUIRE(program.parse_known_args(args).empty());
      } else {
        REQUIRE_NOTHROW(program.parse_args(args));
      }
      REQUIRE(program.get<bool>(first));
      REQUIRE(program.get(second) == "value");
    }
  }
}

TEST_CASE("Compound arguments do not use another prefix's options" *
          test_suite("prefix_chars")) {
  for (const bool known : {false, true}) {
    CAPTURE(known);
    argparse::ArgumentParser program("test");
    program.set_prefix_chars("-+");
    program.add_argument("-a").flag();
    program.add_argument("-b").flag();
    program.add_argument("+a").flag();
    program.add_argument("+b").flag();
    if (known) {
      REQUIRE(program.parse_known_args({"test", "+ab"}).empty());
    } else {
      REQUIRE_NOTHROW(program.parse_args({"test", "+ab"}));
    }
    REQUIRE(program.get<bool>("+a"));
    REQUIRE(program.get<bool>("+b"));
    REQUIRE_FALSE(program.is_used("-a"));
    REQUIRE_FALSE(program.is_used("-b"));
  }
}

TEST_CASE("Unknown custom compound arguments remain unknown" *
          test_suite("prefix_chars")) {
  for (const bool known : {false, true}) {
    CAPTURE(known);
    argparse::ArgumentParser program("test");
    program.set_prefix_chars("-+");
    program.add_argument("-a").flag();
    program.add_argument("-b").flag();
    if (known) {
      const auto unknown = program.parse_known_args({"test", "+ab"});
      REQUIRE((unknown == std::vector<std::string>{"+ab"}));
    } else {
      REQUIRE_THROWS_AS(program.parse_args({"test", "+ab"}),
                        std::runtime_error);
    }
    REQUIRE_FALSE(program.is_used("-a"));
    REQUIRE_FALSE(program.is_used("-b"));
  }
}

TEST_CASE("Exact custom option names take precedence over compounds" *
          test_suite("prefix_chars")) {
  for (const bool known : {false, true}) {
    CAPTURE(known);
    argparse::ArgumentParser program("test");
    program.set_prefix_chars("+");
    program.add_argument("+a").flag();
    program.add_argument("+b").flag();
    program.add_argument("+ab").flag();
    if (known) {
      REQUIRE(program.parse_known_args({"test", "+ab"}).empty());
    } else {
      REQUIRE_NOTHROW(program.parse_args({"test", "+ab"}));
    }
    REQUIRE(program.get<bool>("+ab"));
    REQUIRE_FALSE(program.get<bool>("+a"));
    REQUIRE_FALSE(program.get<bool>("+b"));
  }
}
