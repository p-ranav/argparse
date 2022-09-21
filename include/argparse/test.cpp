#include "argparse.hpp"
#include <cassert>

int main(int argc, char *argv[]) {
  argparse::ArgumentParser program("test");
  program.add_argument("--foo").implicit_value(true).default_value(false);
  program.add_argument("bar");

  auto unknown_args =
    program.parse_known_args({"test", "--foo", "--badger", "BAR", "spam"});

  assert(program.get<bool>("--foo") == true);
  assert(program.get<std::string>("bar") == std::string{"BAR"});
  assert((unknown_args == std::vector<std::string>{"--badger", "spam"}));
}
