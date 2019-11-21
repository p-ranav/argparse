#include <doctest.hpp>
#include <argparse.hpp>

DOCTEST_TEST_CASE("Parse required arguments which are not set and don't have default value. [required_arguments]") {
	argparse::ArgumentParser program("test");
	program.add_argument("--output", "-o").required();
	REQUIRE_THROWS(program.parse_args({ "./main" }));
}

DOCTEST_TEST_CASE("Parse required arguments which are set as empty value and don't have default value. [required_arguments]") {
	argparse::ArgumentParser program("test");
	program.add_argument("--output", "-o").required();
	REQUIRE_THROWS(program.parse_args({ "./main", "-o" }));
}

DOCTEST_TEST_CASE("Parse required arguments which are set as some value and don't have default value. [required_arguments]") {
	argparse::ArgumentParser program("test");
	program.add_argument("--output", "-o").required();
	program.parse_args({ "./main", "-o", "filename" });
	REQUIRE(program.get("--output") == "filename");
	REQUIRE(program.get("-o") == "filename");
}

DOCTEST_TEST_CASE("Parse required arguments which are not set and have default value. [required_arguments]") {
	argparse::ArgumentParser program("test");
	program.add_argument("--output", "-o").required().default_value(std::string("filename"));
	program.parse_args({ "./main" });
	REQUIRE(program.get("--output") == "filename");
	REQUIRE(program.get("-o") == "filename");
}

DOCTEST_TEST_CASE("Parse required arguments which are set as empty and have default value. [required_arguments]") {
	argparse::ArgumentParser program("test");
	program.add_argument("--output", "-o").required().default_value(std::string("filename"));
	REQUIRE_THROWS(program.parse_args({ "./main", "-o" }));
}

DOCTEST_TEST_CASE("Parse required arguments which are set as some value and have default value. [required_arguments]") {
	argparse::ArgumentParser program("test");
	program.add_argument("--output", "-o").required().default_value(std::string("filename"));
	program.parse_args({ "./main", "-o", "anotherfile" });
	REQUIRE(program.get("--output") == "anotherfile");
	REQUIRE(program.get("-o") == "anotherfile");
}