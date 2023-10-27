#ifdef WITH_MODULE
import argparse;
#else
#include <argparse/argparse.hpp>
#endif
#include <doctest.hpp>

#include <optional>
#include <sstream>

using doctest::test_suite;

TEST_CASE("Users can format help message" * test_suite("help")) {
  argparse::ArgumentParser program("test");

  SUBCASE("Simple arguments") {
    program.add_argument("input").help("positional input");
    program.add_argument("-c").help("optional input");
  }
  SUBCASE("Default values") {
    program.add_argument("-a").default_value(42);
    program.add_argument("-b").default_value(4.4e-7);
    program.add_argument("-c")
        .default_value(std::vector<int>{1, 2, 3, 4, 5})
        .nargs(5);
    program.add_argument("-d").default_value("I am a string");
    program.add_argument("-e").default_value(std::optional<float>{});
    program.add_argument("-f").default_value(false);
  }
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

TEST_CASE("Users can disable default -h/--help" * test_suite("help")) {
  argparse::ArgumentParser program("test", "1.0",
                                   argparse::default_arguments::version);
  REQUIRE_THROWS_AS(program.parse_args({"test", "-h"}), std::runtime_error);
}

TEST_CASE("Users can replace default -h/--help" * test_suite("help")) {
  argparse::ArgumentParser program("test", "1.0",
                                   argparse::default_arguments::version);
  std::stringstream buffer;
  program.add_argument("-h", "--help")
      .action([&](const auto &) { buffer << program; })
      .default_value(false)
      .implicit_value(true)
      .nargs(0);

  REQUIRE(buffer.str().empty());
  program.parse_args({"test", "--help"});
  REQUIRE_FALSE(buffer.str().empty());
}

TEST_CASE("Multiline help message alignment") {
  // '#' is used at the beginning of each help message line to simplify testing.
  // It is important to ensure that this character doesn't appear elsewhere in
  // the test case. Default arguments (e.g., -h/--help, -v/--version) are not
  // included in this test.
  argparse::ArgumentParser program("program");
  program.add_argument("INPUT1").help(
      "#This is the first line of help message.\n"
      "#And this is the second line of help message.");
  program.add_argument("program_input2").help("#There is only one line.");
  program.add_argument("-p", "--prog_input3")
      .help(
          R"(#Lorem ipsum dolor sit amet, consectetur adipiscing elit.
#Sed ut perspiciatis unde omnis iste natus error sit voluptatem
#accusantium doloremque laudantium, totam rem aperiam...)");
  program.add_argument("--verbose").default_value(false).implicit_value(true);

  std::ostringstream stream;
  stream << program;
  std::istringstream iss(stream.str());

  auto help_message_start = std::string::npos;
  std::string line;
  while (std::getline(iss, line)) {
    // Find the position of '#', which indicates the start of the help message
    // line
    auto pos = line.find('#');

    if (pos == std::string::npos) {
      continue;
    }

    if (help_message_start == std::string::npos) {
      help_message_start = pos;
    } else {
      REQUIRE(pos == help_message_start);
    }
  }

  // Make sure we have at least one help message
  REQUIRE(help_message_start != -1);
}
