// SPDX-License-Identifier: MIT

#include <argparse/argparse.hpp>

int main(int argc, char *argv[]) {
  argparse::ArgumentParser program("compiler");

  program.add_argument("files").remaining();

  try {
    program.parse_args(argc, argv);
  } catch (const std::exception &err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    return 1;
  }

  try {
    auto files = program.get<std::vector<std::string>>("files");
    std::cout << files.size() << " files provided" << std::endl;
    for (auto &file : files)
      std::cout << file << std::endl;
  } catch (const std::logic_error &) {
    std::cout << "No files provided" << std::endl;
  }
}