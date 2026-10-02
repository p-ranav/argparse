#ifdef WITH_MODULE
import argparse;
#else
#include <argparse/argparse.hpp>
#endif
#include <doctest.hpp>

#include <string>
#include <vector>

using doctest::test_suite;

TEST_CASE("Preserve unknown arguments across subparser dispatch" *
          test_suite("parse_known_args")) {
  argparse::ArgumentParser program("test");
  program.add_argument("--output");
  argparse::ArgumentParser command("run");
  command.add_argument("--count").scan<'i', int>();
  program.add_subparser(command);

  SUBCASE("Unknown arguments only before the subcommand") {
    auto unknown = program.parse_known_args({"test", "--extra", "value", "run"});
    REQUIRE((unknown == std::vector<std::string>{"--extra", "value"}));
  }
  SUBCASE("Unknown arguments only after the subcommand") {
    auto unknown = program.parse_known_args({"test", "run", "--extra", "value"});
    REQUIRE((unknown == std::vector<std::string>{"--extra", "value"}));
  }
  SUBCASE("Preserve order and duplicates around known arguments") {
    auto unknown = program.parse_known_args(
        {"test", "--extra", "before", "--output", "out.txt", "run",
         "--extra", "after", "--count", "3"});
    REQUIRE((unknown == std::vector<std::string>{"--extra", "before",
                                               "--extra", "after"}));
    REQUIRE(program.get<std::string>("--output") == "out.txt");
    REQUIRE(command.get<int>("--count") == 3);
  }
  SUBCASE("Preserve unknown compound option before the subcommand") {
    auto unknown = program.parse_known_args({"test", "-xy", "run", "tail"});
    REQUIRE((unknown == std::vector<std::string>{"-xy", "tail"}));
  }
  SUBCASE("Known arguments alone produce no leftovers") {
    auto unknown = program.parse_known_args(
        {"test", "--output", "out.txt", "run", "--count", "3"});
    REQUIRE(unknown.empty());
    REQUIRE(program.get<std::string>("--output") == "out.txt");
    REQUIRE(command.get<int>("--count") == 3);
  }
  SUBCASE("argc and argv preserve both levels") {
    const char *argv[] = {"test", "--parent", "run", "--child"};
    auto unknown = program.parse_known_args(4, argv);
    REQUIRE((unknown == std::vector<std::string>{"--parent", "--child"}));
  }
  REQUIRE(program.is_subcommand_used("run"));
}

TEST_CASE("Preserve unknown arguments across nested subcommands" *
          test_suite("parse_known_args")) {
  argparse::ArgumentParser program("test"), command("run"), nested("leaf");
  nested.add_argument("--known");
  command.add_subparser(nested);
  program.add_subparser(command);
  auto unknown = program.parse_known_args(
      {"test", "--parent", "P", "run", "--child", "C", "leaf",
       "--known", "K", "--leaf", "L"});
  REQUIRE((unknown == std::vector<std::string>{"--parent", "P", "--child", "C",
                                             "--leaf", "L"}));
  REQUIRE(nested.get<std::string>("--known") == "K");
  REQUIRE(program.is_subcommand_used("run"));
  REQUIRE(command.is_subcommand_used("leaf"));
}

TEST_CASE("Subcommand errors still propagate when parent has unknown arguments" *
          test_suite("parse_known_args")) {
  argparse::ArgumentParser program("test"), command("run");
  command.add_argument("--count").scan<'i', int>();
  program.add_subparser(command);
  REQUIRE_THROWS_AS(program.parse_known_args(
                        {"test", "--unknown", "run", "--count", "invalid"}),
                    std::invalid_argument);
}

TEST_CASE("parse_args remains strict across subparser dispatch" *
          test_suite("parse_known_args")) {
  argparse::ArgumentParser program("test"), command("run");
  program.add_subparser(command);
  SUBCASE("Unknown parent option") {
    REQUIRE_THROWS_AS(program.parse_args({"test", "--parent-extra", "run"}),
                      std::runtime_error);
  }
  SUBCASE("Unknown child option") {
    REQUIRE_THROWS_AS(program.parse_args({"test", "run", "--child-extra"}),
                      std::runtime_error);
  }
}

TEST_CASE("Parse empty argument vector without exceptions" *
          test_suite("parse_known_args")) {
  argparse::ArgumentParser program("test");

  auto unknown_args = program.parse_known_args(std::vector<std::string>{});
  REQUIRE(unknown_args.empty());
}

TEST_CASE("Parse unknown optional and positional arguments without exceptions" *
          test_suite("parse_known_args")) {
  argparse::ArgumentParser program("test");
  program.add_argument("--foo").implicit_value(true).default_value(false);
  program.add_argument("bar");

  SUBCASE("Parse unknown optional and positional arguments") {
    auto unknown_args =
        program.parse_known_args({"test", "--foo", "--badger", "BAR", "spam"});
    REQUIRE((unknown_args == std::vector<std::string>{"--badger", "spam"}));
    REQUIRE(program.get<bool>("--foo") == true);
    REQUIRE(program.get<std::string>("bar") == std::string{"BAR"});
  }

  SUBCASE("Parse unknown compound arguments") {
    auto unknown_args = program.parse_known_args({"test", "-jc", "BAR"});
    REQUIRE((unknown_args == std::vector<std::string>{"-jc"}));
    REQUIRE(program.get<bool>("--foo") == false);
    REQUIRE(program.get<std::string>("bar") == std::string{"BAR"});
  }
}

TEST_CASE("Parse unknown optional and positional arguments in subparsers "
          "without exceptions" *
          test_suite("parse_known_args")) {
  argparse::ArgumentParser program("test");
  program.add_argument("--output");

  argparse::ArgumentParser command_1("add");
  command_1.add_argument("file").nargs(2);

  argparse::ArgumentParser command_2("clean");
  command_2.add_argument("--fullclean")
      .default_value(false)
      .implicit_value(true);

  program.add_subparser(command_1);
  program.add_subparser(command_2);

  SUBCASE("Parse unknown optional argument") {
    auto unknown_args =
        program.parse_known_args({"test", "add", "--badger", "BAR", "spam"});
    REQUIRE(program.is_subcommand_used("add") == true);
    REQUIRE((command_1.get<std::vector<std::string>>("file") ==
             std::vector<std::string>{"BAR", "spam"}));
    REQUIRE((unknown_args == std::vector<std::string>{"--badger"}));
  }

  SUBCASE("Parse unknown positional argument") {
    auto unknown_args =
        program.parse_known_args({"test", "add", "FOO", "BAR", "spam"});
    REQUIRE(program.is_subcommand_used("add") == true);
    REQUIRE((command_1.get<std::vector<std::string>>("file") ==
             std::vector<std::string>{"FOO", "BAR"}));
    REQUIRE((unknown_args == std::vector<std::string>{"spam"}));
  }

  SUBCASE("Parse unknown positional and optional arguments") {
    auto unknown_args = program.parse_known_args(
        {"test", "add", "--verbose", "FOO", "5", "BAR", "-jn", "spam"});
    REQUIRE(program.is_subcommand_used("add") == true);
    REQUIRE((command_1.get<std::vector<std::string>>("file") ==
             std::vector<std::string>{"FOO", "5"}));
    REQUIRE((unknown_args ==
             std::vector<std::string>{"--verbose", "BAR", "-jn", "spam"}));
  }

  SUBCASE("Parse unknown positional and optional arguments 2") {
    auto unknown_args =
        program.parse_known_args({"test", "clean", "--verbose", "FOO", "5",
                                  "BAR", "--fullclean", "-jn", "spam"});
    REQUIRE(program.is_subcommand_used("clean") == true);
    REQUIRE(command_2.get<bool>("--fullclean") == true);
    REQUIRE((unknown_args == std::vector<std::string>{"--verbose", "FOO", "5",
                                                      "BAR", "-jn", "spam"}));
  }
}
