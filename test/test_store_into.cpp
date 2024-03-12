#ifdef WITH_MODULE
import argparse;
#else
#include <argparse/argparse.hpp>
#endif
#include <doctest.hpp>

using doctest::test_suite;

TEST_CASE("Test store_into(bool), flag not specified" *
          test_suite("store_into")) {
  argparse::ArgumentParser program("test");
  bool flag;
  program.add_argument("--flag").store_into(flag);

  program.parse_args({"./test.exe"});
  REQUIRE(flag == false);
}

TEST_CASE("Test store_into(bool), flag specified" *
          test_suite("store_into")) {
  argparse::ArgumentParser program("test");
  bool flag;
  program.add_argument("--flag").store_into(flag);

  program.parse_args({"./test.exe", "--flag"});
  REQUIRE(flag == true);
}

TEST_CASE("Test store_into(int), no default value, non specified" *
          test_suite("store_into")) {
  argparse::ArgumentParser program("test");
  int res = -1;
  program.add_argument("--int-opt").store_into(res);

  program.parse_args({"./test.exe"});
  REQUIRE(res == -1);
}

TEST_CASE("Test store_into(int), default value, non specified" *
          test_suite("store_into")) {
  argparse::ArgumentParser program("test");
  int res = -1;
  program.add_argument("--int-opt").default_value(3).store_into(res);

  program.parse_args({"./test.exe"});
  REQUIRE(res == 3);
}

TEST_CASE("Test store_into(int), default value, specified" *
          test_suite("store_into")) {
  argparse::ArgumentParser program("test");
  int res = -1;
  program.add_argument("--int-opt").default_value(3).store_into(res);

  program.parse_args({"./test.exe", "--int-opt", "5"});
  REQUIRE(res == 5);
}

TEST_CASE("Test store_into(double), no default value, non specified" *
          test_suite("store_into")) {
  argparse::ArgumentParser program("test");
  double res = -1;
  program.add_argument("--double-opt").store_into(res);

  program.parse_args({"./test.exe"});
  REQUIRE(res == -1);
}

TEST_CASE("Test store_into(double), default value, non specified" *
          test_suite("store_into")) {
  argparse::ArgumentParser program("test");
  double res = -1;
  program.add_argument("--double-opt").default_value(3.5).store_into(res);

  program.parse_args({"./test.exe"});
  REQUIRE(res == 3.5);
}

TEST_CASE("Test store_into(double), default value, specified" *
          test_suite("store_into")) {
  argparse::ArgumentParser program("test");
  double res = -1;
  program.add_argument("--double-opt").default_value(3.5).store_into(res);

  program.parse_args({"./test.exe", "--double-opt", "5.5"});
  REQUIRE(res == 5.5);
}

TEST_CASE("Test store_into(string), no default value, non specified" *
          test_suite("store_into")) {
  argparse::ArgumentParser program("test");
  std::string res = "init";
  program.add_argument("--str-opt").store_into(res);

  program.parse_args({"./test.exe"});
  REQUIRE(res == "init");
}

TEST_CASE("Test store_into(string), default value, non specified" *
          test_suite("store_into")) {
  argparse::ArgumentParser program("test");
  std::string res;
  program.add_argument("--str-opt").default_value("default").store_into(res);

  program.parse_args({"./test.exe"});
  REQUIRE(res == "default");
}

TEST_CASE("Test store_into(string), default value, specified" *
          test_suite("store_into")) {
  argparse::ArgumentParser program("test");
  std::string res;
  program.add_argument("--str-opt").default_value("default").store_into(res);

  program.parse_args({"./test.exe", "--str-opt", "foo"});
  REQUIRE(res == "foo");
}

TEST_CASE("Test store_into(vector of string), no default value, non specified" *
          test_suite("store_into")) {
  argparse::ArgumentParser program("test");
  std::vector<std::string> res;
  program.add_argument("--strvector-opt").append().store_into(res);

  program.parse_args({"./test.exe"});
  REQUIRE(res == std::vector<std::string>{});
}

TEST_CASE("Test store_into(vector of string), default value, non specified" *
          test_suite("store_into")) {
  argparse::ArgumentParser program("test");
  std::vector<std::string> res;
  program.add_argument("--strvector-opt").append().default_value(
      std::vector<std::string>{"a", "b"}).store_into(res);

  program.parse_args({"./test.exe"});
  REQUIRE(res == std::vector<std::string>{"a", "b"});
}

TEST_CASE("Test store_into(vector of string), default value, specified" *
          test_suite("store_into")) {
  argparse::ArgumentParser program("test");
  std::vector<std::string> res;
  program.add_argument("--strvector-opt").append().default_value(
      std::vector<std::string>{"a", "b"}).store_into(res);

  program.parse_args({"./test.exe", "--strvector-opt", "foo", "--strvector-opt", "bar"});
  REQUIRE(res == std::vector<std::string>{"foo", "bar"});
}

TEST_CASE("Test store_into(vector of string), default value, multi valued, specified" *
          test_suite("store_into")) {
  argparse::ArgumentParser program("test");
  std::vector<std::string> res;
  program.add_argument("--strvector-opt").nargs(2).default_value(
      std::vector<std::string>{"a", "b"}).store_into(res);

  program.parse_args({"./test.exe", "--strvector-opt", "foo", "bar"});
  REQUIRE(res == std::vector<std::string>{"foo", "bar"});
}
