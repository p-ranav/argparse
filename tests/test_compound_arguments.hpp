#pragma once
#include <catch.hpp>
#include <argparse.hpp>

TEST_CASE("Parse compound toggle arguments with implicit values", "[compound_arguments]") {
  argparse::ArgumentParser program("test");
  program.add_argument("-a")
    .default_value(false)
    .implicit_value(true);

  program.add_argument("-u")
    .default_value(false)
    .implicit_value(true);

  program.add_argument("-x")
    .default_value(false)
    .implicit_value(true);

  program.parse_args({ "./test.exe", "-aux" });
  REQUIRE(program.get<bool>("-a") == true);
  REQUIRE(program.get<bool>("-u") == true);
  REQUIRE(program.get<bool>("-x") == true);
}

TEST_CASE("Parse compound toggle arguments with implicit values and nargs", "[compound_arguments]") {
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

  program.add_argument("--input_files")
    .nargs(3);

  program.parse_args({ "./test.exe", "-abc", "3.14", "2.718", "--input_files",
    "a.txt", "b.txt", "c.txt" });
  REQUIRE(program.get<bool>("-a") == true);
  REQUIRE(program.get<bool>("-b") == true);
  auto c = program.get<std::vector<float>>("-c");
  REQUIRE(c.size() == 2);
  REQUIRE(c[0] == 3.14f);
  REQUIRE(c[1] == 2.718f);
  auto input_files = program.get<std::vector<std::string>>("--input_files");
  REQUIRE(input_files.size() == 3);
  REQUIRE(input_files[0] == "a.txt");
  REQUIRE(input_files[1] == "b.txt");
  REQUIRE(input_files[2] == "c.txt");
}

TEST_CASE("Parse compound toggle arguments with implicit values and nargs and other positional arguments", "[compound_arguments]") {
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

  program.add_argument("--input_files")
    .nargs(3);

  program.parse_args({ "./test.exe", "1", "-abc", "3.14", "2.718", "2", "--input_files",
    "a.txt", "b.txt", "c.txt", "3" });

  REQUIRE(program.get<bool>("-a") == true);
  REQUIRE(program.get<bool>("-b") == true);
  auto c = program.get<std::vector<float>>("-c");
  REQUIRE(c.size() == 2);
  REQUIRE(c[0] == 3.14f);
  REQUIRE(c[1] == 2.718f);
  auto input_files = program.get<std::vector<std::string>>("--input_files");
  REQUIRE(input_files.size() == 3);
  REQUIRE(input_files[0] == "a.txt");
  REQUIRE(input_files[1] == "b.txt");
  REQUIRE(input_files[2] == "c.txt");
  auto numbers = program.get<std::vector<int>>("numbers");
  REQUIRE(numbers.size() == 3);
  REQUIRE(numbers[0] == 1);
  REQUIRE(numbers[1] == 2);
  REQUIRE(numbers[2] == 3);
  auto numbers_list = program.get<std::list<int>>("numbers");
  REQUIRE(numbers.size() == 3);
  REQUIRE(argparse::get_from_list(numbers_list, 0) == 1);
  REQUIRE(argparse::get_from_list(numbers_list, 1) == 2);
  REQUIRE(argparse::get_from_list(numbers_list, 2) == 3);
}