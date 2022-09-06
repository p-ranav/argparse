#include <argparse/argparse.hpp>
#include <doctest.hpp>

using doctest::test_suite;

TEST_CASE("Parse approximate options with no proximity (default)" *
          test_suite("approximate_arguments")) {
  argparse::ArgumentParser program("test");
  program.add_argument("--config").nargs(1);
  program.add_argument("--build").nargs(1);
  REQUIRE_THROWS(
      program.parse_args({"test", "--confi", "test", "--builo", "dev"}));
}

TEST_CASE("Parse approximate options with proximity of 1" *
          test_suite("approximate_arguments")) {
  argparse::ArgumentParser program("test");
  program.set_proximity(1);
  program.add_argument("--config").nargs(1);
  program.add_argument("--build").nargs(1);

  program.parse_args({"test", "--confi", "test", "--buil", "dev"});
  REQUIRE(program.get<std::string>("--config") == "test");
  REQUIRE(program.get<std::string>("--build") == "dev");
  REQUIRE_THROWS(
      program.parse_args({"test", "--conf", "test", "--bilud", "dev"}));
}

TEST_CASE("Parse approximate options with proximity of 2" *
          test_suite("approximate_arguments")) {
  // Deletion
  {
    argparse::ArgumentParser program("test");
    program.set_proximity(2);
    program.add_argument("--config").nargs(1);
    program.add_argument("--build").nargs(1);
    program.parse_args({"test", "--confi", "test", "--bui", "dev"});
    REQUIRE(program.get<std::string>("--config") == "test");
    REQUIRE(program.get<std::string>("--build") == "dev");
  }

  // Insertion
  {
    argparse::ArgumentParser program("test");
    program.set_proximity(2);
    program.add_argument("--config").nargs(1);
    program.add_argument("--build").nargs(1);
    program.parse_args({"test", "--configg", "test", "--builldd", "dev"});
    REQUIRE(program.get<std::string>("--config") == "test");
    REQUIRE(program.get<std::string>("--build") == "dev");
  }

  // Transposition
  {
    argparse::ArgumentParser program("test");
    program.set_proximity(2);
    program.add_argument("--config").nargs(1);
    program.add_argument("--build").nargs(1);
    program.parse_args({"test", "--confgi", "test", "--biudl", "dev"});
    REQUIRE(program.get<std::string>("--config") == "test");
    REQUIRE(program.get<std::string>("--build") == "dev");
  }

  // Substitution
  {
    argparse::ArgumentParser program("test");
    program.set_proximity(2);
    program.add_argument("--config").nargs(1);
    program.add_argument("--build").nargs(1);
    program.parse_args({"test", "--confug", "test", "--biold", "dev"});
    REQUIRE(program.get<std::string>("--config") == "test");
    REQUIRE(program.get<std::string>("--build") == "dev");
  }
}

TEST_CASE("Parse approximate options with proximity of 5" *
          test_suite("approximate_arguments")) {
  argparse::ArgumentParser program("test");
  program.set_proximity(5);
  program.add_argument("--abcdefghijklmnop").nargs(1);
  program.parse_args({"test", "--badcfehgjiklmnop", "test"});
  REQUIRE(program.get<std::string>("--abcdefghijklmnop") == "test");
}