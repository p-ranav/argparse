# Argument Parser

## Highlights

* Simply include <argparse.hpp>
* Requires C++17
* MIT License

## Examples

### Positional Arguments

```cpp
argparse::ArgumentParser program("main");

program.add_argument("input");
program.add_argument("output");

program.parse_args({"./main", "rocket.msh", "thrust_profile.csv"});

std::string input = program.get("input");     // "rocket.msh"
std::string output = program.get("output");   // "thrust_profile.csv"
```

### Construct Objects from Arguments with ```.action```

```cpp
argparse::ArgumentParser program("json_test");

program.add_argument("config")
  .action([](const std::string& value) {
    // read a JSON file
    std::ifstream stream(value);
    nlohmann::json config_json;
    stream >> config_json;
    return config_json;
  });

program.parse_args({"./test", "config.json"});

nlohmann::json config = program.get<nlohmann::json>("config");
```

### Optional Arguments

```cpp
argparse::ArgumentParser program("main");

program.add_argument("--config")
  .help("configuration file")
  .default_value(std::string("config.json"));

program.add_argument("-n", "--num_iterations")
  .help("The list of input files")
  .action([](const std::string& value) { return std::stoi(value); });

program.parse_args({"./main", "-n", "36"});

std::string config = program.get("--config");   // "config.json"
int num_iterations = program.get<int>("-n");    // 36
```

### Vector of Arguments

```cpp
argparse::ArgumentParser program("main");

program.add_argument("--input_files")
  .help("The list of input files")
  .nargs(3);

program.parse_args({"./main", "--input_files", "config.yml", "System.xml"});

auto files = program.get<std::vector<std::string>>("--input_files");  // {"config.yml", "System.xml"}
```

### Toggle Arguments

```cpp
argparse::ArgumentParser program("test");

program.add_argument("--verbose", "-v")
  .default_value(false)
  .implicit_value(true);

program.parse_args({ "./main", "--verbose" });

auto a = program.get<bool>("--verbose");  // true
```

### Compound Arguments

```cpp
argparse::ArgumentParser program("test");

program.add_argument("-a")
  .default_value(false)
  .implicit_value(true);

program.add_argument("-b")
  .default_value(false)
  .implicit_value(true);

program.add_argument("-c")
  .nargs(2)
  .action([](const std::string& value) { return std::stof(value); });

program.parse_args({ "./main", "-abc", "3.14", "2.718" });

auto a = program.get<bool>("-a");                // true
auto b = program.get<bool>("-b");                // true
auto c = program.get<std::vector<float>>("-c");  // {3.14f, 2.718f}
```

### Positional Arguments with Compound Toggle Arguments

```cpp
argparse::ArgumentParser program("test");

program.add_argument("numbers")
  .nargs(3)
  .action([](const std::string& value) { return std::stoi(value); });

program.add_argument("-a")
  .default_value(false)
  .implicit_value(true);

program.add_argument("-b")
  .default_value(false)
  .implicit_value(true);

program.add_argument("-c")
  .nargs(2)
  .action([](const std::string& value) { return std::stof(value); });

program.add_argument("--files")
  .nargs(3);

program.parse_args({ "./test.exe", "1", "-abc", "3.14", "2.718", "2", "--files",
  "a.txt", "b.txt", "c.txt", "3" });

auto numbers = program.get<std::vector<int>>("numbers");        // {1, 2, 3}
auto a = program.get<bool>("-a");                               // true
auto b = program.get<bool>("-b");                               // true
auto c = program.get<std::vector<float>>("-c");                 // {3.14f, 2.718f}
auto files = program.get<std::vector<std::string>>("--files");  // {"a.txt", "b.txt", "c.txt"}
```
