#ifdef WITH_MODULE
import argparse;
#else
#include <argparse/argparse.hpp>
#endif
#include "doctest.hpp"

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
  program.add_argument("--verbose").flag();

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

TEST_CASE("Exclusive arguments, only") {
    argparse::ArgumentParser program("program");
    auto &group = program.add_mutually_exclusive_group();
    group.add_argument("-a").flag();
    group.add_argument("-b").flag();
    REQUIRE(program.usage() == "Usage: program [--help] [--version] [[-a]|[-b]]");
}

TEST_CASE("Exclusive arguments, several groups") {
    argparse::ArgumentParser program("program");
    auto &group = program.add_mutually_exclusive_group();
    group.add_argument("-a").flag();
    group.add_argument("-b").flag();
    auto &group2 = program.add_mutually_exclusive_group();
    group2.add_argument("-c").flag();
    group2.add_argument("-d").flag();
    REQUIRE(program.usage() == "Usage: program [--help] [--version] [[-a]|[-b]] [[-c]|[-d]]");
}

TEST_CASE("Exclusive arguments, several groups, in between arg") {
    argparse::ArgumentParser program("program");
    auto &group = program.add_mutually_exclusive_group();
    group.add_argument("-a").flag();
    group.add_argument("-b").flag();
    program.add_argument("-X").flag();
    auto &group2 = program.add_mutually_exclusive_group();
    group2.add_argument("-c").flag();
    group2.add_argument("-d").flag();
    REQUIRE(program.usage() == "Usage: program [--help] [--version] [[-a]|[-b]] [-X] [[-c]|[-d]]");
}

TEST_CASE("Argument repeatable") {
    argparse::ArgumentParser program("program");
    program.add_argument("-a").flag().append();
    REQUIRE(program.usage() == "Usage: program [--help] [--version] [-a]...");

    std::ostringstream s;
    s << program;
    // std::cout << "DEBUG:" << s.str() << std::endl;
    REQUIRE(s.str().find("  -a             [may be repeated]") != std::string::npos);
}

TEST_CASE("Argument with nargs(2) and metavar <x> <y>") {
    argparse::ArgumentParser program("program");
    program.add_argument("-foo").metavar("<x> <y>").nargs(2);
    REQUIRE(program.usage() == "Usage: program [--help] [--version] [-foo <x> <y>]");
}

TEST_CASE("add_group help") {
    argparse::ArgumentParser program("program");
    program.add_argument("-a").flag().help("help_a");
    program.add_group("Advanced options");
    program.add_argument("-b").flag().help("help_b");
    REQUIRE(program.usage() == "Usage: program [--help] [--version] [-a] [-b]");

    std::ostringstream s;
    s << program;
    // std::cout << "DEBUG:" << s.str() << std::endl;
    REQUIRE(s.str().find(
        "  -a             help_a \n"
        "\n"
        "Advanced options (detailed usage):\n"
        "  -b             help_b") != std::string::npos);
}

TEST_CASE("multiline usage, several groups") {
    argparse::ArgumentParser program("program");
    program.set_usage_max_line_width(80);
    program.add_argument("-a").flag().help("help_a");
    program.add_group("Advanced options");
    program.add_argument("-b").flag().help("help_b");
    // std::cout << "DEBUG:" << program.usage() << std::endl;
    REQUIRE(program.usage() ==
        "Usage: program [--help] [--version] [-a]\n"
        "\n"
        "Advanced options:\n"
        "               [-b]");
}

TEST_CASE("multiline usage, no break on mutex") {
    argparse::ArgumentParser program("program");
    program.set_usage_max_line_width(80);
    program.set_usage_break_on_mutex();
    program.add_argument("--quite-long-option-name").flag();
    auto &group = program.add_mutually_exclusive_group();
    group.add_argument("-a").flag();
    group.add_argument("-b").flag();
    program.add_argument("-c").flag();
    program.add_argument("--another-one").flag();
    program.add_argument("-d").flag();
    program.add_argument("--yet-another-long-one").flag();
    program.add_argument("--will-go-on-new-line").flag();
    // std::cout << "DEBUG:" << program.usage() << std::endl;
    REQUIRE(program.usage() ==
        "Usage: program [--help] [--version] [--quite-long-option-name]\n"
        "               [[-a]|[-b]]\n"
        "               [-c] [--another-one] [-d] [--yet-another-long-one]\n"
        "               [--will-go-on-new-line]");
}

TEST_CASE("multiline usage, break on mutex") {
    argparse::ArgumentParser program("program");
    program.set_usage_max_line_width(80);
    program.add_argument("--quite-long-option-name").flag();
    auto &group = program.add_mutually_exclusive_group();
    group.add_argument("-a").flag();
    group.add_argument("-b").flag();
    program.add_argument("-c").flag();
    program.add_argument("--another-one").flag();
    program.add_argument("-d").flag();
    program.add_argument("--yet-another-long-one").flag();
    program.add_argument("--will-go-on-new-line").flag();
    program.add_usage_newline();
    program.add_argument("--on-a-dedicated-line").flag();
    // std::cout << "DEBUG:" << program.usage() << std::endl;
    REQUIRE(program.usage() ==
        "Usage: program [--help] [--version] [--quite-long-option-name] [[-a]|[-b]] [-c]\n"
        "               [--another-one] [-d] [--yet-another-long-one]\n"
        "               [--will-go-on-new-line]\n"
        "               [--on-a-dedicated-line]");
}
