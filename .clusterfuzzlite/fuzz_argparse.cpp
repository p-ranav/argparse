#include <argparse/argparse.hpp>
#include <fuzzer/FuzzedDataProvider.h>
#include <string>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  FuzzedDataProvider fdp(data, size);

  int args_to_generate = fdp.ConsumeIntegralInRange<int>(1, 10);
  std::vector<std::string> fuzz_args;
  for (int i = 0; i < args_to_generate; i++) {
    fuzz_args.push_back(fdp.ConsumeRandomLengthString());
  }

  // Ensure none of the strings have sequences that cause exit:
  // "-h", "--help", "-v", "--version"
  for (int i = 0; i < args_to_generate; i++) {
    if (fuzz_args[i].find("-h") != std::string::npos ||
        fuzz_args[i].find("-v") != std::string::npos) {
      return 0;
    }
  }

  argparse::ArgumentParser program("test");
  program.add_argument("--config");
  program.add_argument("--test");
  program.add_argument("--fuzzval");
  program.add_argument("--param");
  try {
    program.parse_args(fuzz_args);
  } catch (...) {
  }

  return 0;
}