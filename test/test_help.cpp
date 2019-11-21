#include <doctest.hpp>
#include <argparse.hpp>

DOCTEST_TEST_CASE("Users can format help message [help]") {
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
