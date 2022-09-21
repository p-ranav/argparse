#include <doctest.hpp>
#include <argparse/argparse.hpp>
#include <cmath>

using doctest::test_suite;

TEST_CASE("Add subparsers" * test_suite("subparsers")) {
  argparse::ArgumentParser program("test");
  program.add_argument("--output");

  argparse::ArgumentParser command_1("add");
  command_1.add_argument("file").nargs(2);

  argparse::ArgumentParser command_2("clean");

  program.add_subparser(command_1);
  program.add_subparser(command_2);

  program.parse_args({ "test", "--output", "thrust_profile.csv" });
  REQUIRE(program.get("--output") == "thrust_profile.csv");
}

TEST_CASE("Parse subparser command" * test_suite("subparsers")) {
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

  SUBCASE("command 1") {
    program.parse_args({ "test", "add", "file1.txt", "file2.txt" });
    REQUIRE(command_1.is_used("file"));
    REQUIRE((command_1.get<std::vector<std::string>>("file") == std::vector<std::string>{"file1.txt", "file2.txt"}));
  }

  SUBCASE("command 2") {
    program.parse_args({ "test", "clean", "--fullclean" });
    REQUIRE(command_2.get<bool>("--fullclean") == true);
  }
}

TEST_CASE("Parse subparser command with optional argument" * test_suite("subparsers")) {
  argparse::ArgumentParser program("test");
  program.add_argument("--verbose")
    .default_value(false)
    .implicit_value(true);

  argparse::ArgumentParser command_1("add");
  command_1.add_argument("file");

  argparse::ArgumentParser command_2("clean");
  command_2.add_argument("--fullclean")
    .default_value(false)
    .implicit_value(true);

  program.add_subparser(command_1);
  program.add_subparser(command_2);

  SUBCASE("Optional argument BEFORE subcommand") {
    program.parse_args({ "test", "--verbose", "clean", "--fullclean" });
    REQUIRE(program.get<bool>("--verbose") == true);
    REQUIRE(command_2.get<bool>("--fullclean") == true);
  }

  SUBCASE("Optional argument AFTER subcommand") {
    REQUIRE_THROWS_WITH_AS(program.parse_args({ "test", "clean", "--fullclean", "--verbose" }),
        "Unknown argument: --verbose", std::runtime_error);
  }
}

TEST_CASE("Parse git commands" * test_suite("subparsers")) {
  argparse::ArgumentParser program("git");

  argparse::ArgumentParser add_command("add");
  add_command.add_argument("files")
    .remaining();

  argparse::ArgumentParser commit_command("commit");
  commit_command.add_argument("-a")
    .default_value(false)
    .implicit_value(true);

  commit_command.add_argument("-m");

  argparse::ArgumentParser catfile_command("cat-file");
  catfile_command.add_argument("-t");
  catfile_command.add_argument("-p");

  argparse::ArgumentParser submodule_command("submodule");
  argparse::ArgumentParser submodule_update_command("update");
  submodule_update_command.add_argument("--init")
    .default_value(false)
    .implicit_value(true);
  submodule_update_command.add_argument("--recursive")
    .default_value(false)
    .implicit_value(true);
  submodule_command.add_subparser(submodule_update_command);

  program.add_subparser(add_command);
  program.add_subparser(commit_command);
  program.add_subparser(catfile_command);
  program.add_subparser(submodule_command);

  SUBCASE("git add") {
    program.parse_args({ "git", "add", "main.cpp", "foo.hpp", "foo.cpp" });
    REQUIRE((add_command.get<std::vector<std::string>>("files") == std::vector<std::string>{"main.cpp", "foo.hpp", "foo.cpp"}));
  }

  SUBCASE("git commit") {
    program.parse_args({ "git", "commit", "-am", "Initial commit" });
    REQUIRE(commit_command.get<bool>("-a") == true);
    REQUIRE(commit_command.get<std::string>("-m") == std::string{"Initial commit"});
  }

  SUBCASE("git cat-file -t") {
    program.parse_args({ "git", "cat-file", "-t", "3739f5" });
    REQUIRE(catfile_command.get<std::string>("-t") == std::string{"3739f5"});
  }

  SUBCASE("git cat-file -p") {
    program.parse_args({ "git", "cat-file", "-p", "3739f5" });
    REQUIRE(catfile_command.get<std::string>("-p") == std::string{"3739f5"});
  }

  SUBCASE("git submodule update") {
    program.parse_args({ "git", "submodule", "update" });
  }

  SUBCASE("git submodule update --init") {
    program.parse_args({ "git", "submodule", "update", "--init" });
    REQUIRE(submodule_update_command.get<bool>("--init") == true);
    REQUIRE(submodule_update_command.get<bool>("--recursive") == false);
  }

  SUBCASE("git submodule update --recursive") {
    program.parse_args({ "git", "submodule", "update", "--recursive" });
    REQUIRE(submodule_update_command.get<bool>("--recursive") == true);
  }

  SUBCASE("git submodule update --init --recursive") {
    program.parse_args({ "git", "submodule", "update", "--init", "--recursive" });
    REQUIRE(submodule_update_command.get<bool>("--init") == true);
    REQUIRE(submodule_update_command.get<bool>("--recursive") == true);
  }
}