#include <argparse/argparse.hpp>
#include <doctest.hpp>

using doctest::test_suite;

TEST_CASE("ArgumentParser in bool context" *
          test_suite("argument_parser")) {
  argparse::ArgumentParser program("test");
  program.add_argument("cases").remaining();

  program.parse_args({"test"});
  REQUIRE_FALSE(program);

  program.parse_args({"test", "one", "two"});
  REQUIRE(program);
}

TEST_CASE("With subparsers in bool context" * test_suite("argument_parser")) {
  argparse::ArgumentParser program("test");

  argparse::ArgumentParser cmd_fly("fly");
  cmd_fly.add_argument("plane");

  argparse::ArgumentParser cmd_soar("soar");
  cmd_soar.add_argument("direction");

  program.add_subparser(cmd_fly);
  program.add_subparser(cmd_soar);

  program.parse_args({"test", "fly", "glider"});
  REQUIRE(program);
  REQUIRE(cmd_fly);
  REQUIRE_FALSE(cmd_soar);
}

TEST_CASE("Parsers remain false with unknown arguments" *
           test_suite("argument_parser")) {
  argparse::ArgumentParser program("test");

  argparse::ArgumentParser cmd_build("build");
  cmd_build.add_argument("--file").nargs(1);

  argparse::ArgumentParser cmd_run("run");
  cmd_run.add_argument("--file").nargs(1);

  program.add_subparser(cmd_build);
  program.add_subparser(cmd_run);

  auto unknowns =
      program.parse_known_args({"test", "badger", "--add-meal", "grubs"});
  REQUIRE_FALSE(program);
  REQUIRE_FALSE(cmd_build);
  REQUIRE_FALSE(cmd_run);
}

TEST_CASE("Multi-level parsers match subparser bool" *
           test_suite("argument_parser")) {
  argparse::ArgumentParser program("test");

  argparse::ArgumentParser cmd_cook("cook");
  cmd_cook.add_argument("--temperature");

  argparse::ArgumentParser cmd_cook_boil("boil");
  cmd_cook_boil.add_argument("--rate");

  argparse::ArgumentParser cmd_cook_boil_stir("stir");
  cmd_cook_boil_stir.add_argument("--rate");

  argparse::ArgumentParser cmd_wash("wash");

  program.add_subparser(cmd_cook);
  cmd_cook.add_subparser(cmd_cook_boil);
  cmd_cook_boil.add_subparser(cmd_cook_boil_stir);

  program.add_subparser(cmd_wash);

  auto unknowns = program.parse_known_args(
      {"test", "cook", "boil", "stir", "--rate", "fast"});

  REQUIRE(program);
  REQUIRE(cmd_cook);
  REQUIRE(cmd_cook_boil);
  REQUIRE(cmd_cook_boil_stir);
  REQUIRE_FALSE(cmd_wash);
}
