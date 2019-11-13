/*
  __ _ _ __ __ _ _ __   __ _ _ __ ___  ___
 / _` | '__/ _` | '_ \ / _` | '__/ __|/ _ \ Argument Parser for Modern C++
| (_| | | | (_| | |_) | (_| | |  \__ \  __/ http://github.com/p-ranav/argparse
 \__,_|_|  \__, | .__/ \__,_|_|  |___/\___|
           |___/|_|

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2019 Pranav Srinivas Kumar <pranav.srinivas.kumar@gmail.com>.

Permission is hereby  granted, free of charge, to any  person obtaining a copy
of this software and associated  documentation files (the "Software"), to deal
in the Software  without restriction, including without  limitation the rights
to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#pragma once
#include <algorithm>
#include <any>
#include <functional>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <memory>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace argparse {

namespace details { // namespace for helper methods

template <typename... Ts> struct is_container_helper {};

template <typename T, typename _ = void>
struct is_container : std::false_type {};

template <> struct is_container<std::string> : std::false_type {};

template <typename T>
struct is_container<
    T,
    std::conditional_t<false,
                       is_container_helper<typename T::value_type,
                                           decltype(std::declval<T>().begin()),
                                           decltype(std::declval<T>().end()),
                                           decltype(std::declval<T>().size())>,
                       void>> : public std::true_type {};

template <typename T>
static constexpr bool is_container_v = is_container<T>::value;

template <typename T>
using enable_if_container = std::enable_if_t<is_container_v<T>, T>;

template <typename T>
using enable_if_not_container = std::enable_if_t<!is_container_v<T>, T>;
} // namespace

class ArgumentParser;

class Argument {
  friend class ArgumentParser;
  friend auto operator<<(std::ostream &, ArgumentParser const &)
      -> std::ostream &;

public:
  Argument() = default;

  template <typename... Args>
  explicit Argument(Args... args)
      : mNames({std::move(args)...}), mIsOptional((is_optional(args) || ...)) {
    std::sort(
        mNames.begin(), mNames.end(), [](const auto &lhs, const auto &rhs) {
          return lhs.size() == rhs.size() ? lhs < rhs : lhs.size() < rhs.size();
        });
  }

  Argument &help(std::string aHelp) {
    mHelp = std::move(aHelp);
    return *this;
  }

  Argument &default_value(std::any aDefaultValue) {
    mDefaultValue = std::move(aDefaultValue);
    return *this;
  }

  Argument &required() {
    mIsRequired = true;
    return *this;
  }

  Argument &implicit_value(std::any aImplicitValue) {
    mImplicitValue = std::move(aImplicitValue);
    mNumArgs = 0;
    return *this;
  }

  Argument &action(std::function<std::any(const std::string &)> aAction) {
    mAction = std::move(aAction);
    return *this;
  }

  Argument &nargs(size_t aNumArgs) {
    mNumArgs = aNumArgs;
    return *this;
  }

  template <typename Iterator>
  Iterator consume(Iterator start, Iterator end, std::string usedName = {}) {
    if (mIsUsed) {
      throw std::runtime_error("Duplicate argument");
    }
    mIsUsed = true;
    mUsedName = std::move(usedName);
    if (mNumArgs == 0) {
      mValues.emplace_back(mImplicitValue);
      return start;
    } else if (mNumArgs <= static_cast<size_t>(std::distance(start, end))) {
      end = std::next(start, mNumArgs);
      if (std::any_of(start, end, Argument::is_optional)) {
        throw std::runtime_error("optional argument in parameter sequence");
      }
      std::transform(start, end, std::back_inserter(mValues), mAction);
      return end;
    } else if (mDefaultValue.has_value()) {
      return start;
    } else {
      throw std::runtime_error("Too few arguments");
    }
  }

  /*
   * @throws std::runtime_error if argument values are not valid
   */
  void validate() const {
    if (mIsOptional) {
      if (mIsUsed && mValues.size() != mNumArgs && !mDefaultValue.has_value()) {
        std::stringstream stream;
        stream << mUsedName << ": expected " << mNumArgs
               << " argument(s). " << mValues.size() << " provided.";
        throw std::runtime_error(stream.str());
      } else {
        // TODO: check if an implicit value was programmed for this argument
        if (!mIsUsed && !mDefaultValue.has_value() && mIsRequired) {
          std::stringstream stream;
          stream << mNames[0] << ": required.";
          throw std::runtime_error(stream.str());
        }
        if (mIsUsed && mIsRequired && mValues.size() == 0) {
          std::stringstream stream;
          stream << mUsedName << ": no value provided.";
          throw std::runtime_error(stream.str());
        }
      }
    } else {
      if (mValues.size() != mNumArgs && !mDefaultValue.has_value()) {
        std::stringstream stream;
        stream << mUsedName << ": expected " << mNumArgs
               << " argument(s). " << mValues.size() << " provided.";
        throw std::runtime_error(stream.str());
      }
    }
  }

  size_t get_arguments_length() const {
    return std::accumulate(std::begin(mNames), std::end(mNames), size_t(0),
                           [](const auto &sum, const auto &s) {
                             return sum + s.size() +
                                    1; // +1 for space between names
                           });
  }

  friend std::ostream &operator<<(std::ostream &stream,
                                  const Argument &argument) {
    std::stringstream nameStream;
    std::copy(std::begin(argument.mNames), std::end(argument.mNames),
              std::ostream_iterator<std::string>(nameStream, " "));
    stream << nameStream.str() << "\t" << argument.mHelp;
    if (argument.mIsRequired)
      stream << "[Required]";
    stream << "\n";
    return stream;
  }

  template <typename T> bool operator!=(const T &aRhs) const {
    return !(*this == aRhs);
  }

  /*
   * Entry point for template non-container types
   * @throws std::logic_error in case of incompatible types
   */
  template <typename T>
  std::enable_if_t<!details::is_container_v<T>, bool> operator==(const T &aRhs) const {
    return get<T>() == aRhs;
  }

  /*
   * Template specialization for containers
   * @throws std::logic_error in case of incompatible types
   */
  template <typename T>
  std::enable_if_t<details::is_container_v<T>, bool> operator==(const T &aRhs) const {
    using ValueType = typename T::value_type;
    auto tLhs = get<T>();
    if (tLhs.size() != aRhs.size())
      return false;
    else {
      return std::equal(std::begin(tLhs), std::end(tLhs), std::begin(aRhs),
                        [](const auto &lhs, const auto &rhs) {
                          return std::any_cast<const ValueType &>(lhs) == rhs;
                        });
    }
  }

private:
  static bool is_integer(const std::string &aValue) {
    if (aValue.empty() ||
        ((!isdigit(aValue[0])) && (aValue[0] != '-') && (aValue[0] != '+')))
      return false;
    char *tPtr;
    strtol(aValue.c_str(), &tPtr, 10);
    return (*tPtr == 0);
  }

  static bool is_float(const std::string &aValue) {
    std::istringstream tStream(aValue);
    float tFloat;
    // noskipws considers leading whitespace invalid
    tStream >> std::noskipws >> tFloat;
    // Check the entire string was consumed
    // and if either failbit or badbit is set
    return tStream.eof() && !tStream.fail();
  }

  // If an argument starts with "-" or "--", then it's optional
  static bool is_optional(const std::string &aName) {
    return (!aName.empty() && aName[0] == '-' && !is_integer(aName) &&
            !is_float(aName));
  }

  static bool is_positional(const std::string &aName) {
    return !is_optional(aName);
  }

  /*
   * Getter for template non-container types
   * @throws std::logic_error in case of incompatible types
   */
  template <typename T> details::enable_if_not_container<T> get() const {
    if (!mValues.empty()) {
      return std::any_cast<T>(mValues.front());
    }
    if (mDefaultValue.has_value()) {
      return std::any_cast<T>(mDefaultValue);
    }
    throw std::logic_error("No value provided");
  }

  /*
   * Getter for container types
   * @throws std::logic_error in case of incompatible types
   */
  template <typename CONTAINER> details::enable_if_container<CONTAINER> get() const {
    using ValueType = typename CONTAINER::value_type;
    CONTAINER tResult;
    if (!mValues.empty()) {
      std::transform(
          std::begin(mValues), std::end(mValues), std::back_inserter(tResult),
          [](const auto &value) { return std::any_cast<ValueType>(value); });
      return tResult;
    }
    if (mDefaultValue.has_value()) {
      const auto &tDefaultValues =
          std::any_cast<const CONTAINER &>(mDefaultValue);
      std::transform(std::begin(tDefaultValues), std::end(tDefaultValues),
                     std::back_inserter(tResult), [](const auto &value) {
                       return std::any_cast<ValueType>(value);
                     });
      return tResult;
    }
    throw std::logic_error("No value provided");
  }

  std::vector<std::string> mNames;
  std::string mUsedName;
  std::string mHelp;
  std::any mDefaultValue;
  std::any mImplicitValue;
  std::function<std::any(const std::string &)> mAction =
      [](const std::string &aValue) { return aValue; };
  std::vector<std::any> mValues;
  std::vector<std::string> mRawValues;
  size_t mNumArgs = 1;
  bool mIsOptional = false;
  bool mIsRequired = false;
  bool mIsUsed = false; // relevant for optional arguments. True if used by user

public:
  static constexpr auto mHelpOption = "-h";
  static constexpr auto mHelpOptionLong = "--help";
};

class ArgumentParser {
public:
  explicit ArgumentParser(std::string aProgramName = {})
      : mProgramName(std::move(aProgramName)) {
    add_argument(Argument::mHelpOption, Argument::mHelpOptionLong)
        .help("show this help message and exit")
        .nargs(0)
        .default_value(false)
        .implicit_value(true);
  }

  // Parameter packing
  // Call add_argument with variadic number of string arguments
  template <typename... Targs> Argument &add_argument(Targs... Fargs) {
    std::shared_ptr<Argument> tArgument =
        std::make_shared<Argument>(std::move(Fargs)...);

    if (tArgument->mIsOptional)
      mOptionalArguments.emplace_back(tArgument);
    else
      mPositionalArguments.emplace_back(tArgument);

    for (const auto &mName : tArgument->mNames) {
      mArgumentMap.insert_or_assign(mName, tArgument);
    }
    return *tArgument;
  }

  // Parameter packed add_parents method
  // Accepts a variadic number of ArgumentParser objects
  template <typename... Targs> void add_parents(Targs... Fargs) {
    const auto tNewParentParsers = {Fargs...};
    for (const auto &tParentParser : tNewParentParsers) {
      const auto &tPositionalArguments = tParentParser.mPositionalArguments;
      std::copy(std::begin(tPositionalArguments),
                std::end(tPositionalArguments),
                std::back_inserter(mPositionalArguments));

      const auto &tOptionalArguments = tParentParser.mOptionalArguments;
      std::copy(std::begin(tOptionalArguments), std::end(tOptionalArguments),
                std::back_inserter(mOptionalArguments));

      const auto &tArgumentMap = tParentParser.mArgumentMap;
      for (const auto &[tKey, tValue] : tArgumentMap) {
        mArgumentMap.insert_or_assign(tKey, tValue);
      }
    }
    std::move(std::begin(tNewParentParsers), std::end(tNewParentParsers),
              std::back_inserter(mParentParsers));
  }

  /* Call parse_args_internal - which does all the work
   * Then, validate the parsed arguments
   * This variant is used mainly for testing
   * @throws std::runtime_error in case of any invalid argument
   */
  void parse_args(const std::vector<std::string> &aArguments) {
    parse_args_internal(aArguments);
    parse_args_validate();
  }

  /* Main entry point for parsing command-line arguments using this
   * ArgumentParser
   * @throws std::runtime_error in case of any invalid argument
   */
  void parse_args(int argc, const char *const argv[]) {
    std::vector<std::string> arguments;
    std::copy(argv, argv + argc, std::back_inserter(arguments));
    parse_args(arguments);
  }

  /* Getter enabled for all template types other than std::vector and std::list
   * @throws std::logic_error in case of an invalid argument name
   * @throws std::logic_error in case of incompatible types
   */
  template <typename T = std::string> T get(const std::string &aArgumentName) {
    auto tIterator = mArgumentMap.find(aArgumentName);
    if (tIterator != mArgumentMap.end()) {
      return tIterator->second->get<T>();
    }
    throw std::logic_error("No such argument");
  }

  /* Indexing operator. Return a reference to an Argument object
   * Used in conjuction with Argument.operator== e.g., parser["foo"] == true
   * @throws std::logic_error in case of an invalid argument name
   */
  Argument &operator[](const std::string &aArgumentName) {
    auto tIterator = mArgumentMap.find(aArgumentName);
    if (tIterator != mArgumentMap.end()) {
      return *(tIterator->second);
    }
    throw std::logic_error("No such argument");
  }

  // Print help message
  friend auto operator<<(std::ostream &stream, const ArgumentParser &parser)
      -> std::ostream & {
    if (auto sen = std::ostream::sentry(stream)) {
      stream.setf(std::ios_base::left);
      stream << "Usage: " << parser.mProgramName << " [options] ";
      size_t tLongestArgumentLength = parser.get_length_of_longest_argument();

      for (const auto &argument : parser.mPositionalArguments) {
        stream << argument->mNames.front() << " ";
      }
      stream << "\n\n";

      if (!parser.mPositionalArguments.empty())
        stream << "Positional arguments:\n";

      for (const auto &mPositionalArgument : parser.mPositionalArguments) {
        stream.width(tLongestArgumentLength);
        stream << *mPositionalArgument;
      }

      if (!parser.mOptionalArguments.empty())
        stream << (parser.mPositionalArguments.empty() ? "" : "\n")
               << "Optional arguments:\n";

      for (const auto &mOptionalArgument : parser.mOptionalArguments) {
        stream.width(tLongestArgumentLength);
        stream << *mOptionalArgument;
      }
    }

    return stream;
  }

  // Format help message
  auto help() const -> std::ostringstream {
    std::ostringstream out;
    out << *this;
    return out;
  }

  // Printing the one and only help message
  // I've stuck with a simple message format, nothing fancy.
  std::string print_help() {
    auto out = help();
    std::cout << out.rdbuf();
    return out.str();
  }

private:
  /*
   * @throws std::runtime_error in case of any invalid argument
   */
  void parse_args_internal(const std::vector<std::string> &aArguments) {
    if (mProgramName.empty() && !aArguments.empty()) {
      mProgramName = aArguments.front();
    }
    auto end = std::end(aArguments);
    auto positionalArgumentIt = std::begin(mPositionalArguments);
    for (auto it = std::next(std::begin(aArguments)); it != end;) {
      const auto &tCurrentArgument = *it;
      if (tCurrentArgument == Argument::mHelpOption ||
          tCurrentArgument == Argument::mHelpOptionLong) {
        throw std::runtime_error("help called");
      }
      if (Argument::is_positional(tCurrentArgument)) {
        if (positionalArgumentIt == std::end(mPositionalArguments)) {
          throw std::runtime_error(
              "Maximum number of positional arguments exceeded");
        }
        auto tArgument = *(positionalArgumentIt++);
        it = tArgument->consume(it, end);
      } else if (auto tIterator = mArgumentMap.find(tCurrentArgument);
                 tIterator != mArgumentMap.end()) {
        auto tArgument = tIterator->second;
        it = tArgument->consume(std::next(it), end, tCurrentArgument);
      } else if (const auto &tCompoundArgument = tCurrentArgument;
                 tCompoundArgument.size() > 1 && tCompoundArgument[0] == '-' &&
                 tCompoundArgument[1] != '-') {
        ++it;
        for (size_t j = 1; j < tCompoundArgument.size(); j++) {
          auto tCurrentArgument = std::string{'-', tCompoundArgument[j]};
          if (auto tIterator = mArgumentMap.find(tCurrentArgument);
              tIterator != mArgumentMap.end()) {
            auto tArgument = tIterator->second;
            it = tArgument->consume(it, end, tCurrentArgument);
          } else {
            throw std::runtime_error("Unknown argument");
          }
        }
      } else {
        throw std::runtime_error("Unknown argument");
      }
    }
  }

  /*
   * @throws std::runtime_error in case of any invalid argument
   */
  void parse_args_validate() {
    // Check if all arguments are parsed
    std::for_each(std::begin(mArgumentMap), std::end(mArgumentMap),
                  [](const auto &argPair) {
                    const auto &tArgument = argPair.second;
                    tArgument->validate();
                  });
  }

  // Used by print_help.
  size_t get_length_of_longest_argument() const {
    if (mArgumentMap.empty())
      return 0;
    std::vector<size_t> argumentLengths(mArgumentMap.size());
    std::transform(std::begin(mArgumentMap), std::end(mArgumentMap),
                   std::begin(argumentLengths), [](const auto &argPair) {
                     const auto &tArgument = argPair.second;
                     return tArgument->get_arguments_length();
                   });
    return *std::max_element(std::begin(argumentLengths),
                             std::end(argumentLengths));
  }

  std::string mProgramName;
  std::vector<ArgumentParser> mParentParsers;
  std::vector<std::shared_ptr<Argument>> mPositionalArguments;
  std::vector<std::shared_ptr<Argument>> mOptionalArguments;
  std::map<std::string, std::shared_ptr<Argument>> mArgumentMap;
};

#define PARSE_ARGS(parser, argc, argv)                                         \
  try {                                                                        \
    parser.parse_args(argc, argv);                                             \
  } catch (const std::runtime_error &err) {                                    \
    std::cout << err.what() << std::endl;                                      \
    parser.print_help();                                                       \
    exit(0);                                                                   \
  }

} // namespace argparse
