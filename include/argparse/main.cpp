#include "argparse.hpp"

int main(int argc, char* argv[]) {
	argparse::ArgumentParser program;
	program.add_argument("-a", "--number-of-apples");
	program.add_argument("-b", "--bro");
	program.parse_args(argc, argv);
}