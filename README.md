# Argument Parser for Modern C++

## Highlights

* Header-only library
* Requires C++17
* MIT License

## Quick Start

Simply include argparse.hpp and you're good to go.

```cpp
#include <argparse.hpp>
```

To start parsing command-line arguments, create an ```ArgumentParser```. 

```cpp
argparse::ArgumentParser program("program name");
```

Argparse supports a variety of argument types including:
* Positional arguments
* Optional arguments
* Toggle arguments
* Compound arguments

Here's an example of a positional argument:

```cpp
program.add_argument("square")
  .help("display the square of a given integer")
  .action([](const std::string& value) { auto integer = std::stoi(value); return integer * integer; });

program.parse_args(argc, argv);
std::cout << program.get<int>("square") << std::endl;
```

And running the code:

```bash
$ ./main 15
225
```

Here's what's happening:

* The ```add_argument()``` method is used to specify which command-line options the program is willing to accept. In this case, I’ve named it square so that it’s in line with its function.
* Command-line arguments are strings. Inorder to square the argument and print the result, we need to convert this argument to a number. In order to do this, we use the ```.action``` method and provide a lambda function that takes the argument value (std::string) and returns the square of the number it represents. Actions are quite powerful as you will see in later examples. 
* Calling our program now requires us to specify an option.
* The parse_args() method parses the arguments provided, converts our input into an integer and returns the square. 
* We can get the value stored by the parser for a given argument using ```parser.get<T>(key)``` method. 

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

if (program["--verbose'] == true) {   // true
    // enable verbose logging
}
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

### Restricting the set of values for an argument

```cpp
argparse::ArgumentParser program("test");

program.add_argument("input")
  .default_value("baz")
  .action([=](const std::string& value) {
    static const std::vector<std::string> choices = { "foo", "bar", "baz" };
    if (std::find(choices.begin(), choices.end(), value) != choices.end()) {
      return value;
    }
    return std::string{ "baz" };
  });

program.parse_args({ "./test", "fez" });

auto input = program.get("input"); // baz
```
