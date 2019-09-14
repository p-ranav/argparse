<p align="center">
  <img height="100" src="https://i.imgur.com/oDXeMUQ.png" alt="argparse"/>
</p>

<p align="center">
  <img src="https://travis-ci.org/p-ranav/argparse.svg?branch=master" alt="travis"/>
  <a href="https://github.com/p-ranav/argparse/blob/master/LICENSE">
    <img src="https://img.shields.io/badge/License-MIT-yellow.svg" alt="license"/>
  </a>
  <img src="https://img.shields.io/badge/version-1.9-blue.svg?cacheSeconds=2592000" alt="version"/>
</p>

## Highlights

* Single header file
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

To add a new argument, simply call ```.add_argument(...)```. You can provide a variadic list of argument names that you want to group together, e.g., ```-v``` and ```--verbose```

```cpp
program.add_argument("foo");
program.add_argument("-v", "--verbose"); // parameter packing
```

Argparse supports a variety of argument types including positional, optional, and compound arguments. Below you can see how to configure each of these types:

### Positional Arguments

Here's an example of a ***positional argument***:

```cpp
#include <argparse.hpp>

int main(int argc, char *argv[]) {
  argparse::ArgumentParser program("program name");

  program.add_argument("square")
    .help("display the square of a given integer")
    .action([](const std::string& value) { return std::stoi(value); });

  try {
    program.parse_args(argc, argv);
  }
  catch (const std::runtime_error& err) {
    std::cout << err.what() << std::endl;
    program.print_help();
    exit(0);
  }
  
  auto input = program.get<int>("square");
  std::cout << (input * input) << std::endl;

  return 0;
}
```

And running the code:

```bash
$ ./main 15
225
```

Here's what's happening:

* The ```add_argument()``` method is used to specify which command-line options the program is willing to accept. In this case, I’ve named it square so that it’s in line with its function.
* Command-line arguments are strings. Inorder to square the argument and print the result, we need to convert this argument to a number. In order to do this, we use the ```.action``` method and provide a lambda function that tries to convert user input into an integer.
* We can get the value stored by the parser for a given argument using ```parser.get<T>(key)``` method. 

### Optional Arguments

Now, let's look at ***optional arguments***. Optional arguments start with ```-``` or ```--```, e.g., ```--verbose``` or ```-a```. Optional arguments can be placed anywhere in the input sequence. 


```cpp
argparse::ArgumentParser program("test");

program.add_argument("--verbose")
  .help("increase output verbosity")
  .default_value(false)
  .implicit_value(true);

try {
  program.parse_args(argc, argv);
}
catch (const std::runtime_error& err) {
  std::cout << err.what() << std::endl;
  program.print_help();
  exit(0);
}

if (program["--verbose"] == true) {
    std::cout << "Verbosity enabled" << std::endl;
}
```

```bash
$ ./main --verbose
Verbosity enabled
```

Here's what's happening: 
* The program is written so as to display something when --verbose is specified and display nothing when not.
* Since the argument is actually optional, no error is thrown when running the program without ```--verbose```. Note that by using ```.default_value(false)```, if the optional argument isn’t used, it's value is automatically set to false. 
* By using ```.implicit_value(true)```, the user specifies that this option is more of a flag than something that requires a value. When the user provides the --verbose option, it's value is set to true. 

#### Requiring optional arguments

There are scenarios where you would like to make an optional argument ***required***. As discussed above, optional arguments either begin with `-` or `--`. You can make these types of arguments required like so:

```cpp
program.add_argument("-o", "--output")
  .required()
  .help("specify the output file.");
```

If the user does not provide a value for this parameter, an exception is thrown. 

Alternatively, you could provide a default value like so:

```cpp
program.add_argument("-o", "--output")
  .default_value(std::string("-"))
  .required()
  .help("specify the output file.");
```

### Negative Numbers

Optional arguments start with ```-```. Can ```argparse``` handle negative numbers? The answer is yes!

```cpp
argparse::ArgumentParser program;

program.add_argument("integer")
  .help("Input number")
  .action([](const std::string& value) { return std::stoi(value); });
  
program.add_argument("floats")
  .help("Vector of floats")
  .nargs(4)
  .action([](const std::string& value) { return std::stof(value); });

try {
  program.parse_args(argc, argv);
}
catch (const std::runtime_error& err) {
  std::cout << err.what() << std::endl;
  program.print_help();
  exit(0);
}

// Some code to print arguments
```

```bash
$ ./main -5 -1.1 -3.1415 -3.1e2 -4.51329E3
integer : -5
floats  : -1.1 -3.1415 -310 -4513.29
```

As you can see here, ```argparse``` supports negative integers, negative floats and scientific notation. 

### Combining Positional and Optional Arguments

```cpp
argparse::ArgumentParser program("test");

program.add_argument("square")
  .help("display the square of a given number")
  .action([](const std::string& value) { return std::stoi(value); });

program.add_argument("--verbose")
  .default_value(false)
  .implicit_value(true);

try {
  program.parse_args(argc, argv);
}
catch (const std::runtime_error& err) {
  std::cout << err.what() << std::endl;
  program.print_help();
  exit(0);
}

int input = program.get<int>("square");

if (program["--verbose"] == true) {
  std::cout << "The square of " << input << " is " << (input * input) << std::endl;
}
else {
  std::cout << (input * input) << std::endl;
}
```

```bash
$ ./main 4
16

$ ./main 4 --verbose
The square of 4 is 16

$ ./main --verbose 4
The square of 4 is 16
```

### Printing Help

```ArgumentParser.print_help()``` print a help message, including the program usage and information about the arguments registered with the ArgumentParser. For the previous example, here's the default help message:

```
$ ./main --help
Usage: ./main [options] square

Positional arguments:
square         display a square of a given number

Optional arguments:
-h, --help     show this help message and exit
-v, --verbose  enable verbose logging
```

### List of Arguments

ArgumentParser objects usually associate a single command-line argument with a single action to be taken. The ```.nargs``` associates a different number of command-line arguments with a single action. When using ```nargs(N)```, N arguments from the command line will be gathered together into a list.

```cpp
argparse::ArgumentParser program("main");

program.add_argument("--input_files")
  .help("The list of input files")
  .nargs(2);

try {
  program.parse_args(argc, argv);   // Example: ./main --input_files config.yml System.xml
}
catch (const std::runtime_error& err) {
  std::cout << err.what() << std::endl;
  program.print_help();
  exit(0);
}

auto files = program.get<std::vector<std::string>>("--input_files");  // {"config.yml", "System.xml"}
```

```ArgumentParser.get<T>()``` has specializations for ```std::vector``` and ```std::list```. So, the following variant, ```.get<std::list>```, will also work. 

```cpp
auto files = program.get<std::list<std::string>>("--input_files");  // {"config.yml", "System.xml"}
```

Using ```.action```, one can quickly build a list of desired value types from command line arguments. Here's an example:

```cpp
argparse::ArgumentParser program("main");

program.add_argument("--query_point")
  .help("3D query point")
  .nargs(3)
  .default_value(std::vector<double>{0.0, 0.0, 0.0})
  .action([](const std::string& value) { return std::stod(value); });

try {
  program.parse_args(argc, argv); // Example: ./main --query_point 3.5 4.7 9.2
}
catch (const std::runtime_error& err) {
  std::cout << err.what() << std::endl;
  program.print_help();
  exit(0);
}

auto query_point = program.get<std::vector<double>>("--query_point");  // {3.5, 4.7, 9.2}
```

### Compound Arguments

Compound arguments are optional arguments that are combined and provided as a single argument. Example: ```ps -aux```

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
  .default_value(std::vector<float>{0.0f, 0.0f})
  .action([](const std::string& value) { return std::stof(value); });

try {
  program.parse_args(argc, argv);                  // Example: ./main -abc 1.95 2.47
}
catch (const std::runtime_error& err) {
  std::cout << err.what() << std::endl;
  program.print_help();
  exit(0);
}

auto a = program.get<bool>("-a");                  // true
auto b = program.get<bool>("-b");                  // true
auto c = program.get<std::vector<float>>("-c");    // {1.95, 2.47}

/// Some code that prints parsed arguments
```

```bash
$ ./main -ac 3.14 2.718
a = true
b = false
c = {3.14, 2.718}

$ ./main -cb
a = false
b = true
c = {0.0, 0.0}
```

Here's what's happening:
* We have three optional arguments ```-a```, ```-b``` and ```-c```. 
* ```-a``` and ```-b``` are toggle arguments.
* ```-c``` requires 2 floating point numbers from the command-line.
* argparse can handle compound arguments, e.g., ```-abc``` or ```-bac``` or ```-cab```. This only works with short single-character argument names. 
  - ```-a``` and ```-b``` become true.
  - argv is further parsed to identify the inputs mapped to ```-c```.
  - If argparse cannot find any arguments to map to c, then c defaults to {0.0, 0.0} as defined by ```.default_value```


### Parent Parsers

Sometimes, several parsers share a common set of arguments. Rather than repeating the definitions of these arguments, a single parser with all the shared arguments can be added as a parent to another ArgumentParser instance. The ```.add_parents``` method takes a list of ArgumentParser objects, collects all the positional and optional actions from them, and adds these actions to the ArgumentParser object being constructed:

```cpp
argparse::ArgumentParser parent_parser("main");
parent_parser.add_argument("--parent")
  .default_value(0)
  .action([](const std::string& value) { return std::stoi(value); });

argparse::ArgumentParser foo_parser("foo");
foo_parser.add_argument("foo");
foo_parser.add_parents(parent_parser);
foo_parser.parse_args({ "./main", "--parent", "2", "XXX" });   // parent = 2, foo = XXX

argparse::ArgumentParser bar_parser("bar");
bar_parser.add_argument("--bar");
bar_parser.parse_args({ "./main", "--bar", "YYY" });           // bar = YYY
```

Note You must fully initialize the parsers before passing them via ```.add_parents```. If you change the parent parsers after the child parser, those changes will not be reflected in the child.

## Further Examples

### Construct a JSON object from a filename argument

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

try {
  program.parse_args({"./test", "config.json"});
}
catch (const std::runtime_error& err) {
  std::cout << err.what() << std::endl;
  program.print_help();
  exit(0);
}

nlohmann::json config = program.get<nlohmann::json>("config");
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

try {
  program.parse_args(argc, argv);
}
catch (const std::runtime_error& err) {
  std::cout << err.what() << std::endl;
  program.print_help();
  exit(0);
}

auto numbers = program.get<std::vector<int>>("numbers");        // {1, 2, 3}
auto a = program.get<bool>("-a");                               // true
auto b = program.get<bool>("-b");                               // true
auto c = program.get<std::vector<float>>("-c");                 // {3.14f, 2.718f}
auto files = program.get<std::vector<std::string>>("--files");  // {"a.txt", "b.txt", "c.txt"}

/// Some code that prints parsed arguments
```

```bash
$ ./main 1 2 3 -abc 3.14 2.718 --files a.txt b.txt c.txt
numbers = {1, 2, 3}
a = true
b = true
c = {3.14, 2.718}
files = {"a.txt", "b.txt", "c.txt"}
```

### Restricting the set of values for an argument

```cpp
argparse::ArgumentParser program("test");

program.add_argument("input")
  .default_value("baz")
  .action([](const std::string& value) {
    static const std::vector<std::string> choices = { "foo", "bar", "baz" };
    if (std::find(choices.begin(), choices.end(), value) != choices.end()) {
      return value;
    }
    return std::string{ "baz" };
  });

try {
  program.parse_args(argc, argv);
}
catch (const std::runtime_error& err) {
  std::cout << err.what() << std::endl;
  program.print_help();
  exit(0);
}

auto input = program.get("input");
std::cout << input << std::endl;
```

```bash
$ ./main fex
baz
```

## Supported Compilers
* GCC >= 7.0.0
* Clang >= 4.0
* MSVC >= 2017

## Contributing
Contributions are welcome, have a look at the [CONTRIBUTING.md](CONTRIBUTING.md) document for more information.

## License
The project is available under the [MIT](https://opensource.org/licenses/MIT) license.
