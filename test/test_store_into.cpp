#ifdef WITH_MODULE
import argparse;
#else
#include <argparse/argparse.hpp>
#endif
#include <cstdint>
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

// int cases

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

// integral cases

TEST_CASE("Test store_into(uint64_t) with scan" * test_suite("store_into")) {
  argparse::ArgumentParser program("test");
  uint64_t res = 0;
  auto &arg = program.add_argument("foo");
  std::string input;
  uint64_t expected = 0;

  SUBCASE("hexadecimal with prefix") {
    arg.scan<'x', uint64_t>();
    input = "0xABCD";
    expected = 0xABCD;
  }
  SUBCASE("hexadecimal without prefix") {
    arg.scan<'X', uint64_t>();
    input = "ABCD";
    expected = 0xABCD;
  }
  SUBCASE("octal") {
    arg.scan<'o', uint64_t>();
    input = "077";
    expected = 63;
  }
  SUBCASE("binary") {
    arg.scan<'b', uint64_t>();
    input = "0b1010";
    expected = 10;
  }
  SUBCASE("decimal") {
    arg.scan<'u', uint64_t>();
    input = "123";
    expected = 123;
  }
  SUBCASE("automatic base") {
    arg.scan<'i', uint64_t>();
    input = "0xABCD";
    expected = 0xABCD;
  }

  arg.store_into(res);
  program.parse_args({"test", input});
  REQUIRE(res == expected);
  REQUIRE(program.get<uint64_t>("foo") == expected);
  REQUIRE(program.get<std::vector<uint64_t>>("foo").size() == 1);
}

TEST_CASE("Test store_into with scan and actions" * test_suite("store_into")) {
  argparse::ArgumentParser program("test");
  uint64_t res = 0;
  int calls = 0;
  program.add_argument("foo")
      .scan<'x', uint64_t>()
      .action([&](const std::string &) { ++calls; })
      .store_into(res)
      .action([&](const std::string &) { ++calls; });

  program.parse_args({"test", "0xABCD"});
  REQUIRE(res == 0xABCD);
  REQUIRE(program.get<uint64_t>("foo") == res);
  REQUIRE(calls == 2);
}

TEST_CASE("Test store_into with scan rejects invalid input" *
          test_suite("store_into")) {
  argparse::ArgumentParser program("test");
  uint64_t res = 7;
  program.add_argument("foo").scan<'x', uint64_t>().store_into(res);

  SUBCASE("invalid digit") {
    REQUIRE_THROWS_AS(program.parse_args({"test", "0xABCG"}),
                      std::invalid_argument);
  }
  SUBCASE("overflow") {
    REQUIRE_THROWS_AS(program.parse_args({"test", "0x10000000000000000"}),
                      std::range_error);
  }
  REQUIRE(res == 7);
}

TEST_CASE("Test store_into with scan preserves action ordering" *
          test_suite("store_into")) {
  argparse::ArgumentParser program("test");
  uint64_t value = 7;
  uint64_t observed = 0;
  program.add_argument("foo")
      .nargs(1, 2)
      .scan<'d', uint64_t>()
      .action([&](const std::string &) { observed = value; })
      .store_into(value);

  program.parse_args({"test", "123"});
  REQUIRE(observed == 7);
  REQUIRE(value == 123);
  REQUIRE(program.get<std::vector<uint64_t>>("foo") ==
          std::vector<uint64_t>{123});
}

TEST_CASE("Test store_into with scan preserves exception ordering" *
          test_suite("store_into")) {
  argparse::ArgumentParser program("test");
  uint64_t value = 7;
  program.add_argument("foo")
      .scan<'d', uint64_t>()
      .action([](const std::string &) {
        throw std::runtime_error("custom failure");
      })
      .store_into(value);

  REQUIRE_THROWS_WITH_AS(program.parse_args({"test", "123"}), "custom failure",
                         std::runtime_error);
  REQUIRE(value == 7);
}

TEST_CASE("Test store_into with scan preserves multiple values without defaults" *
          test_suite("store_into")) {
  argparse::ArgumentParser program("test");
  uint64_t value = 7;
  auto &arg =
      program.add_argument("--hex").scan<'x', uint64_t>().store_into(value);

  SUBCASE("multiple tokens") {
    arg.nargs(2);
    program.parse_args({"test", "--hex", "0xABCD", "0x1234"});
  }
  SUBCASE("repeated argument") {
    arg.append();
    program.parse_args({"test", "--hex", "0xABCD", "--hex", "0x1234"});
  }
  REQUIRE(value == 0x1234);
  REQUIRE(program.get<std::vector<uint64_t>>("--hex") ==
          std::vector<uint64_t>{0xABCD, 0x1234});
}

TEST_CASE("Test store_into(double) with scan" * test_suite("store_into")) {
  argparse::ArgumentParser program("test");
  double res = 0;
  program.add_argument("foo").scan<'a', double>().store_into(res);

  program.parse_args({"test", "0x1.8p+1"});
  REQUIRE(res == 3.0);
  REQUIRE(program.get<double>("foo") == res);
  REQUIRE(program.get<std::vector<double>>("foo").size() == 1);
}

TEST_CASE("Test store_into with scan preserves defaults and repeated values" *
          test_suite("store_into")) {
  argparse::ArgumentParser program("test");
  uint64_t res = 0;
  program.add_argument("--hex")
      .default_value(uint64_t{7})
      .scan<'x', uint64_t>()
      .store_into(res)
      .append();

  SUBCASE("default") {
    REQUIRE(res == 7);
    program.parse_args({"test"});
    REQUIRE(program.get<uint64_t>("--hex") == 7);
    REQUIRE(res == 7);
  }
  SUBCASE("repeated values") {
    program.parse_args({"test", "--hex", "0xABCD", "--hex", "0x1234"});
    REQUIRE(res == 0x1234);
    REQUIRE(program.get<std::vector<uint64_t>>("--hex") ==
            std::vector<uint64_t>{0xABCD, 0x1234});
  }
}

TEST_CASE("Test store_into preserves independent parsing for different types" *
          test_suite("store_into")) {
  argparse::ArgumentParser program("test");
  double res = 0;
  program.add_argument("foo")
      .nargs(1, 2)
      .scan<'d', int>()
      .store_into(res);

  program.parse_args({"test", "12"});
  REQUIRE(program.get<int>("foo") == 12);
  REQUIRE(res == 12.0);
}

TEST_CASE("Test store_into(uint8_t), no default value, non specified" *
          test_suite("store_into")) {
  argparse::ArgumentParser program("test");
  uint8_t res = 55;
  program.add_argument("--int-opt").store_into(res);

  program.parse_args({"./test.exe"});
  REQUIRE(res == 55);
}

TEST_CASE("Test store_into(uint8_t), default value, non specified" *
          test_suite("store_into")) {
  argparse::ArgumentParser program("test");
  uint8_t res = 55;
  program.add_argument("--int-opt").default_value((uint8_t)3).store_into(res);

  program.parse_args({"./test.exe"});
  REQUIRE(res == 3);
}

TEST_CASE("Test store_into(uint8_t), default value, specified" *
          test_suite("store_into")) {
  argparse::ArgumentParser program("test");
  uint8_t res = 55;
  program.add_argument("--int-opt").default_value((uint8_t)3).store_into(res);

  program.parse_args({"./test.exe", "--int-opt", "5"});
  REQUIRE(res == 5);
}

// Double cases

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

// Float cases

TEST_CASE("Test store_into(float), no default value, non specified" *
          test_suite("store_into")) {
  argparse::ArgumentParser program("test");
  float res = -1.0f;
  program.add_argument("--float-opt").store_into(res);

  program.parse_args({"./test.exe"});
  REQUIRE(res == -1.0f);
}

TEST_CASE("Test store_into(float), default value, non specified" *
          test_suite("store_into")) {
  argparse::ArgumentParser program("test");
  float res = -1.0f;
  program.add_argument("--float-opt").default_value(3.5f).store_into(res);

  program.parse_args({"./test.exe"});
  REQUIRE(res == 3.5f);
}

TEST_CASE("Test store_into(float), default value, specified" *
          test_suite("store_into")) {
  argparse::ArgumentParser program("test");
  float res = -1.0f;
  program.add_argument("--float-opt").default_value(3.5f).store_into(res);

  program.parse_args({"./test.exe", "--float-opt", "5.5"});
  REQUIRE(res == 5.5f);
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

TEST_CASE("Test store_into(vector of int), no default value, non specified" *
          test_suite("store_into")) {
  argparse::ArgumentParser program("test");
  std::vector<int> res;
  program.add_argument("--intvector-opt").append().store_into(res);

  program.parse_args({"./test.exe"});
  REQUIRE(res == std::vector<int>{});
}

TEST_CASE("Test store_into(vector of int), default value, non specified" *
          test_suite("store_into")) {
  argparse::ArgumentParser program("test");
  std::vector<int> res;
  program.add_argument("--intvector-opt").append().default_value(
      std::vector<int>{1, 2}).store_into(res);

  program.parse_args({"./test.exe"});
  REQUIRE(res == std::vector<int>{1, 2});
}

TEST_CASE("Test store_into(vector of int), default value, specified" *
          test_suite("store_into")) {
  argparse::ArgumentParser program("test");
  std::vector<int> res;
  program.add_argument("--intvector-opt").append().default_value(
      std::vector<int>{1, 2}).store_into(res);

  program.parse_args({"./test.exe", "--intvector-opt", "3", "--intvector-opt", "4"});
  REQUIRE(res == std::vector<int>{3, 4});
}

TEST_CASE("Test store_into(vector of int), default value, multi valued, specified" *
          test_suite("store_into")) {
  argparse::ArgumentParser program("test");
  std::vector<int> res;
  program.add_argument("--intvector-opt").nargs(2).default_value(
      std::vector<int>{1, 2}).store_into(res);

  program.parse_args({"./test.exe", "--intvector-opt", "3", "4"});
  REQUIRE(res == std::vector<int>{3, 4});
}

TEST_CASE("Test store_into(set of int), default value, multi valued, specified" *
          test_suite("store_into")) {

  {
    argparse::ArgumentParser program("test");
    std::set<int> res;
    program.add_argument("--intset-opt").nargs(2).default_value(
                                                        std::set<int>{1, 2}).store_into(res);

    program.parse_args({"./test.exe", "--intset-opt", "3", "4"});
    REQUIRE(res == std::set<int>{3, 4});
  }

  {
    argparse::ArgumentParser program("test");
    std::set<int> res;
    program.add_argument("--intset-opt").nargs(2).default_value(
                                                     std::set<int>{1, 2}).store_into(res);
    program.parse_args({"./test.exe"});
    REQUIRE(res == std::set<int>{1, 2});
  }
}

TEST_CASE("Test store_into(set of string), default value, multi valued, specified" *
          test_suite("store_into")) {

  {
    argparse::ArgumentParser program("test");
    std::set<std::string> res;
    program.add_argument("--stringset-opt").nargs(2).default_value(
                                                        std::set<std::string>{"1", "2"}).store_into(res);

    program.parse_args({"./test.exe", "--stringset-opt", "3", "4"});
    REQUIRE(res == std::set<std::string>{"3", "4"});
  }

  {
    argparse::ArgumentParser program("test");
    std::set<std::string> res;
    program.add_argument("--stringset-opt").nargs(2).default_value(
                                                        std::set<std::string>{"1", "2"}).store_into(res);
    program.parse_args({"./test.exe"});
    REQUIRE(res == std::set<std::string>{"1", "2"});
  }
}

TEST_CASE("Test store_into(int) still works with a custom action" *
          test_suite("store_into")) {

  GIVEN("an argument with store_into followed by a custom action ") {
    argparse::ArgumentParser program("test");
    int res;
    std::string string_res;
    program.add_argument("--int").store_into(res).action([&](const auto &s) {string_res.append(s);});

    WHEN("the argument is parsed") {
    program.parse_args({"./test.exe", "--int", "3"});
      THEN("the value is stored and the action was executed") {
        REQUIRE(res == 3);
        REQUIRE(string_res == "3");
      }
    }
  }

  GIVEN("an argument with a custom action followed by store_into")
  {
    argparse::ArgumentParser program("test");
    int res;
    std::string string_res;
    program.add_argument("--int").action([&](const auto &s) {string_res.append(s);}).store_into(res);

    WHEN("the argument is parsed") {
    program.parse_args({"./test.exe", "--int", "3"});
      THEN("the value is stored and the action was executed") {
        REQUIRE(res == 3);
        REQUIRE(string_res == "3");
      }
    }
  }
}

