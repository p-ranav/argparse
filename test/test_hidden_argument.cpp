#ifdef WITH_MODULE
import argparse;
#else
#include <argparse/argparse.hpp>
#endif
#include <doctest.hpp>

using doctest::test_suite;

TEST_CASE("Test setting a hidden argument" * test_suite("hidden_argument")) {
  argparse::ArgumentParser program("program");
  program.add_argument("--hidden").flag().hidden();
  program.add_argument("--regular").flag();
  program.add_argument("regular_positional");
  // only makes sense if last and optional...
  program.add_argument("hidden_positional").nargs(0, 1).hidden();

  program.parse_args({"./test.exe", "--hidden", "--regular",
                      "regular_positional_val", "hidden_positional_val"});
  REQUIRE(program.get<bool>("--hidden") == true);
  REQUIRE(program.get<bool>("--regular") == true);
  REQUIRE(program.get<std::string>("regular_positional") ==
          "regular_positional_val");
  REQUIRE(program.get<std::string>("hidden_positional") ==
          "hidden_positional_val");

  REQUIRE(program.usage() ==
          "Usage: program [--help] [--version] [--regular] regular_positional");

  std::ostringstream s;
  s << program;
  // std::cout << "DEBUG:" << s.str() << std::endl;
  REQUIRE(s.str().find("hidden") == std::string::npos);
  REQUIRE(s.str().find("--regular") != std::string::npos);
  REQUIRE(s.str().find("regular_positional") != std::string::npos);
}
