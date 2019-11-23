#include <doctest.hpp>
#include <argparse.hpp>

using doctest::test_suite;

TEST_CASE("Users can format help message" * test_suite("help")) {
  argparse::ArgumentParser program("test");
  program.add_argument("input")
    .help("positional input");
  program.add_argument("-c")
    .help("optional input");

  std::ostringstream s;
  s << program;
  REQUIRE_FALSE(s.str().empty());

  auto msg = program.help().str();
  REQUIRE(msg == s.str());
}

TEST_CASE("Users can override the help options" * test_suite("help")) {
  GIVEN("a program that meant to take -h as a normal option") {
    argparse::ArgumentParser program("test");
    program.add_argument("input");
    program.add_argument("-h").implicit_value('h').default_value('x');

    WHEN("provided -h without fulfilling other requirements") {
      THEN("validation fails") {
        REQUIRE_THROWS_AS(program.parse_args({"test", "-h"}),
                          std::runtime_error);
      }
    }

    WHEN("provided arguments to all parameters") {
      program.parse_args({"test", "-h", "some input"});

      THEN("these parameters get their values") {
        REQUIRE(program["-h"] == 'h');
        REQUIRE(program.get("input") == "some input");
      }
    }
  }
}
