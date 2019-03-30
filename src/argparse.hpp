#include <string>

namespace argparse {

class ArgumentParser {
  public:
    ArgumentParser(const std::string& program_name) :
      program_name_(program_name) {}

  private:
    std::string program_name_;
};

}