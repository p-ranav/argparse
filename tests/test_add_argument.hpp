#include <catch.hpp>
#include <argparse.hpp>

TEST_CASE("Add a simple argument", "[add_argument]") {
  argparse::ArgumentParser program("test");
  program.add_argument("--foo");

  auto arguments = program.get_arguments();
  REQUIRE(arguments.size() == 1);
  REQUIRE(arguments["--foo"] != nullptr);
  REQUIRE(arguments["--foo"]->mNames == std::vector<std::string>{"--foo"});
  REQUIRE(arguments["--foo"]->mHelp == "");
  REQUIRE(arguments["--foo"]->mNumArgs == 1);
  REQUIRE(arguments["--foo"]->mRawValues.size() == 0);
  REQUIRE(arguments["--foo"]->mValues.size() == 0);
}

TEST_CASE("Add a simple argument with help", "[add_argument]") {
  argparse::ArgumentParser program("test");
  program.add_argument("--foo")
    .help("input file");

  auto arguments = program.get_arguments();
  REQUIRE(arguments.size() == 1);
  REQUIRE(arguments["--foo"] != nullptr);
  REQUIRE(arguments["--foo"]->mNames == std::vector<std::string>{"--foo"});
  REQUIRE(arguments["--foo"]->mHelp == "input file");
  REQUIRE(arguments["--foo"]->mNumArgs == 1);
  REQUIRE(arguments["--foo"]->mRawValues.size() == 0);
  REQUIRE(arguments["--foo"]->mValues.size() == 0);
}