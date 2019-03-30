#include <iostream>
#include <argparse.hpp>
using namespace argparse;

int main(int argc, char * argv[]) {
  ArgumentParser program("test");

  program.add_argument("--config")
    .help("Path to input file")
    .default_value([]() { return std::string{"config.yml"}; });

  program.add_argument("-n", "--num_iterations")
    .help("Number of iterations")
    .nargs(2)
    .action([](const std::string& value) { return std::stoi(value); });

  program.add_argument("-v", "--verbose", "VERBOSE")
    .default_value([]() { return true; });

  program.parse_args_2(argc, argv);

  auto config_file = program.get<std::string>("--config");
  auto num_iters = program.get<int>("-n");
  auto verbose = program.get<bool>("-v");

  std::cout << config_file << std::endl;
  std::cout << num_iters << std::endl;
  std::cout << verbose << std::endl;
  return 0;
}