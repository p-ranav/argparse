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
    .action([](const std::string& value) { return std::stoi(value); });

  program.add_argument("-v", "--verbose", "VERBOSE")
    .nargs(0)
    .default_value([]() { return false; });

  program.add_argument("--test_inputs")
    .nargs(3)
    .default_value([]() { return std::vector<int>{ 1, 2, 3 }; })
    .action([](const std::string& value) { return std::stoi(value); });

  program.parse_args(argc, argv);

  auto config_file = program.get<std::string>("--config");
  auto num_iters = program.get<int>("-n");
  auto verbose = program.get<bool>("-v");
  auto test_inputs = program.get_list<int>("--test_inputs");

  std::cout << config_file << std::endl;
  std::cout << num_iters << std::endl;
  std::cout << verbose << std::endl;
  for (auto& input : test_inputs)
    std::cout << input << std::endl;

  return 0;
}