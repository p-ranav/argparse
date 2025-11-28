#ifdef WITH_MODULE
import argparse;
#else
#include <argparse/argparse.hpp>
#endif
#include <doctest.hpp>

using doctest::test_suite;

TEST_CASE("Regression test for issue 413" * test_suite("implicit_values")) {
    argparse::ArgumentParser program;
    bool nicearg = false;

    program.add_argument("--deactivate")
      .help("Disable BFS-based detector ordering and use geometric orientation")
      .default_value(true)
      .implicit_value(false)
      .store_into(nicearg);

    program.parse_args({"test"});
    REQUIRE(nicearg == true);
}

TEST_CASE("Regression test for issue 413 [2]" * test_suite("implicit_values")) {
    argparse::ArgumentParser program;
    bool nicearg = true;

    program.add_argument("--deactivate")
      .help("Disable BFS-based detector ordering and use geometric orientation")
      .default_value(true)
      .implicit_value(false)
      .store_into(nicearg);

    program.parse_args({"test", "--deactivate"});
    REQUIRE(nicearg == false);
}
