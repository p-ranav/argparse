#pragma once
#include <catch.hpp>
#include <argparse.hpp>

TEST_CASE("Parse required arguments which is not set and doesn't have default value.", "[required_arguments]") {
	argparse::ArgumentParser program("test");
	program.add_argument("--output", "-o").required();
	REQUIRE_THROWS(program.parse_args({ "./main" }));
}

TEST_CASE("Parse required arguments without default value which is set as empty value and doesn't have default value.", "[required_arguments]") {
	argparse::ArgumentParser program("test");
	program.add_argument("--output", "-o").required();
	REQUIRE_THROWS(program.parse_args({ "./main", "-o" }));
}

TEST_CASE("Parse required arguments without default value which is set as some value and doesn't have default value.", "[required_arguments]") {
	argparse::ArgumentParser program("test");
	program.add_argument("--output", "-o").required();
	program.parse_args({ "./main", "-o", "filename" });
	REQUIRE(program.get("--output") == "filename");
	REQUIRE(program.get("-o") == "filename");
}

TEST_CASE("Parse required arguments which is not set and has default value.", "[required_arguments]") {
	argparse::ArgumentParser program("test");
	program.add_argument("--output", "-o").required().default_value(std::string("filename"));
	program.parse_args({ "./main" });
	REQUIRE(program.get("--output") == "filename");
	REQUIRE(program.get("-o") == "filename");
}

TEST_CASE("Parse required arguments without default value which is set as empty and has default value.", "[required_arguments]") {
	argparse::ArgumentParser program("test");
	program.add_argument("--output", "-o").required().default_value(std::string("filename"));
	REQUIRE_THROWS(program.parse_args({ "./main", "-o" }));
}

TEST_CASE("Parse required arguments without default value which is set as some value and has default value.", "[required_arguments]") {
	argparse::ArgumentParser program("test");
	program.add_argument("--output", "-o").required().default_value(std::string("filename"));
	program.parse_args({ "./main", "-o", "anotherfile" });
	REQUIRE(program.get("--output") == "anotherfile");
	REQUIRE(program.get("-o") == "anotherfile");
}