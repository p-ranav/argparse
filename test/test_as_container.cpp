#include <argparse/argparse.hpp>
#include <doctest.hpp>

using doctest::test_suite;

TEST_CASE("Get argument with .at()" * test_suite("as_container")) {
  argparse::ArgumentParser program("test");
  auto &dir_arg = program.add_argument("--dir");
  program.at("--dir").default_value(std::string("/home/user"));

  SUBCASE("and default value") {
    program.parse_args({"test"});
    REQUIRE(&(program.at("--dir")) == &dir_arg);
    REQUIRE(program["--dir"] == std::string("/home/user"));
    REQUIRE(program.at("--dir") == std::string("/home/user"));
  }

  SUBCASE("and user-supplied value") {
    program.parse_args({"test", "--dir", "/usr/local/database"});
    REQUIRE(&(program.at("--dir")) == &dir_arg);
    REQUIRE(program["--dir"] == std::string("/usr/local/database"));
    REQUIRE(program.at("--dir") == std::string("/usr/local/database"));
  }

  SUBCASE("with unknown argument") {
    program.parse_args({"test"});
    REQUIRE_THROWS_WITH_AS(program.at("--folder"),
                           "No such argument: --folder", std::logic_error);
  }
}

TEST_CASE("Get subparser with .at()" * test_suite("as_container")) {
  argparse::ArgumentParser program("test");

  argparse::ArgumentParser walk_cmd("walk");
  auto &speed = walk_cmd.add_argument("speed");

  program.add_subparser(walk_cmd);

  SUBCASE("and its argument") {
    program.parse_args({"test", "walk", "4km/h"});
    REQUIRE(&(program.at<argparse::ArgumentParser>("walk")) == &walk_cmd);
    REQUIRE(&(program.at<argparse::ArgumentParser>("walk").at("speed")) == &speed);
    REQUIRE(program.at<argparse::ArgumentParser>("walk").is_used("speed"));
  }

  SUBCASE("with unknown command") {
    program.parse_args({"test"});
    REQUIRE_THROWS_WITH_AS(program.at<argparse::ArgumentParser>("fly"),
                           "No such subparser: fly", std::logic_error);
  }
}
