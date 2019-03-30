# Argument Parser

## Simple Arguments

```cpp
#include <argparse.hpp>

int main(int argc, char *argv[]) {
  argparse::ArgumentParser program("main");
  
  program.add_argument("--config")
    .help("configuration file")
    .default_value(std::string("config.yml"));
    
  program.add_argument("-n", "--num_iterations")
    .help("The list of input files")
    .action([](const std::string& value) { return std::stoi(value); });
    
  program.parse(argc, argv);
  std::string config = program.get("--config");
  int num_iterations = program.get<int>("-n");  
  
  return 0;
}
```

## Parsing a list of arguments

```cpp
#include <argparse.hpp>

int main(int argc, char *argv[]) {
  argparse::ArgumentParser program("main");
  
  program.add_argument("--input_files")
    .help("The list of input files")
    .nargs(3);
    
  program.parse(argc, argv);
  std::vector<std::string> files = program.get<std::vector<std::string>>("--input_files");  
  
  return 0;
}
```
