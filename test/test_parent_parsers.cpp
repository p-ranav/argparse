#ifdef WITH_MODULE
import argparse;
#else
#include <argparse/argparse.hpp>
#endif
#include <doctest.hpp>

using doctest::test_suite;

TEST_CASE("Add parent parsers" * test_suite("parent_parsers")) {
  argparse::ArgumentParser parent_parser("main");
  parent_parser.add_argument("--verbose")
      .default_value(false)
      .implicit_value(true);

  argparse::ArgumentParser child_parser("foo");
  child_parser.add_parents(parent_parser);
  child_parser.parse_args({"./main", "--verbose"});
  REQUIRE(child_parser["--verbose"] == true);
  REQUIRE(parent_parser["--verbose"] == false);
}

TEST_CASE("Add parent to multiple parent parsers" *
          test_suite("parent_parsers")) {
  argparse::ArgumentParser parent_parser("main");
  parent_parser.add_argument("--parent").default_value(0).scan<'i', int>();

  argparse::ArgumentParser foo_parser("foo");
  foo_parser.add_argument("foo");
  foo_parser.add_parents(parent_parser);
  foo_parser.parse_args({"./main", "--parent", "2", "XXX"});
  REQUIRE(foo_parser["--parent"] == 2);
  REQUIRE(foo_parser["foo"] == std::string("XXX"));
  REQUIRE(parent_parser["--parent"] == 0);

  argparse::ArgumentParser bar_parser("bar");
  bar_parser.add_argument("--bar");
  bar_parser.parse_args({"./main", "--bar", "YYY"});
  REQUIRE(bar_parser["--bar"] == std::string("YYY"));
}

TEST_CASE("Parent parsers preserve required mutually exclusive groups" *
          test_suite("parent_parsers")) {
  argparse::ArgumentParser parent_parser("parent", "1.0",
                                         argparse::default_arguments::none);
  auto &group = parent_parser.add_mutually_exclusive_group(true);
  group.add_argument("-a", "--host").flag();
  group.add_argument("--guest").flag();

  argparse::ArgumentParser child_parser("child");
  child_parser.add_parents(parent_parser);

  SUBCASE("a group member is required") {
    REQUIRE_THROWS_AS(child_parser.parse_args({"child"}), std::runtime_error);
  }
  SUBCASE("conflicting members are rejected") {
    REQUIRE_THROWS_AS(child_parser.parse_args({"child", "--host", "--guest"}),
                      std::runtime_error);
  }
  SUBCASE("an alias satisfies the child group independently") {
    REQUIRE_NOTHROW(child_parser.parse_args({"child", "-a"}));
    REQUIRE(child_parser["--host"] == true);
    REQUIRE_FALSE(parent_parser.is_used("--host"));
    REQUIRE_FALSE(parent_parser.is_used("--guest"));
  }
}

TEST_CASE("Inherited mutually exclusive groups outlive their parent" *
          test_suite("parent_parsers")) {
  argparse::ArgumentParser child_parser("child");
  {
    argparse::ArgumentParser parent_parser("parent", "1.0",
                                           argparse::default_arguments::none);
    auto &group = parent_parser.add_mutually_exclusive_group();
    group.add_argument("--host").flag();
    group.add_argument("--guest").flag();
    child_parser.add_parents(parent_parser);
  }

  REQUIRE_THROWS_AS(child_parser.parse_args({"child", "--host", "--guest"}),
                    std::runtime_error);
}
