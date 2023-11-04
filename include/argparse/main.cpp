#include "argparse.hpp"

int main(int argc, char* argv[]) {
	argparse::ArgumentParser program;
	program.add_argument("-a").required();
	program.add_argument("-b", "--bro").required();
	program.parse_args(argc, argv);
}