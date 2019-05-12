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
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <list>
#include <functional>
#include <any>
#include <memory>
#include <type_traits>
#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <numeric>

namespace argparse {

namespace { // anonymous namespace for helper methods - not visible outside this header file
// Some utility structs to check template specialization
template<typename Test, template<typename...> class Ref>
struct is_specialization : std::false_type {};

template<template<typename...> class Ref, typename... Args>
struct is_specialization<Ref<Args...>, Ref> : std::true_type {};

// Check if string (haystack) starts with a substring (needle)
bool starts_with(const std::string& haystack, const std::string& needle) {
  return needle.length() <= haystack.length()
    && std::equal(needle.begin(), needle.end(), haystack.begin());
}
}

class Argument {
  friend class ArgumentParser;
public:
  Argument() = default;

  template <typename ...Args>
  explicit Argument(Args... args)
    : mNames({std::move(args)...})
    , mIsOptional((is_optional(args) || ...))
  {}

  Argument& help(std::string aHelp) {
    mHelp = std::move(aHelp);
    return *this;
  }

  Argument& default_value(std::any aDefaultValue) {
    mDefaultValue = std::move(aDefaultValue);
    return *this;
  }

  Argument& implicit_value(std::any aImplicitValue) {
    mImplicitValue = std::move(aImplicitValue);
    mNumArgs = 0;
    return *this;
  }

  Argument& action(std::function<std::any(const std::string&)> aAction) {
    mAction = std::move(aAction);
    return *this;
  }

  Argument& nargs(size_t aNumArgs) {
    mNumArgs = aNumArgs;
    return *this;
  }

  /*
   * @throws std::runtime_error if argument values are not valid
   */
  void validate() const {
    if (mIsOptional) {
      if (mIsUsed && mValues.size() != mNumArgs && !mDefaultValue.has_value()) {
        std::stringstream stream;
        stream << "error: " << mUsedName << ": expected " << mNumArgs << " argument(s). "
               << mValues.size() << " provided.\n" << std::endl;
        throw std::runtime_error(stream.str());
      }
      else {
        // TODO: check if an implicit value was programmed for this argument
      }
    }
    else {
      if (mValues.size() != mNumArgs) {
        std::stringstream stream;
        stream << "error: " << mUsedName << ": expected " << mNumArgs << " argument(s). "
               << mValues.size() << " provided.\n" << std::endl;
        throw std::runtime_error(stream.str());
      }
    }
  }


  template <typename T>
  bool operator!=(const T& aRhs) const {
    return !(*this == aRhs);
  }

  // Entry point for template types other than std::vector and std::list
  template <typename T>
  typename std::enable_if<!is_specialization<T, std::vector>::value &&
                          !is_specialization<T, std::list>::value, bool>::type
    operator==(const T& aRhs) const {
    return get<T>() == aRhs;
  }

  // Template specialization for std::vector<...>
  template <typename T>
  typename std::enable_if<is_specialization<T, std::vector>::value, bool>::type
  operator==(const T& aRhs) const {
    using ValueType = typename T::value_type;
    T tLhs = get<T>();
    if (tLhs.size() != aRhs.size())
      return false;
    else {
      return std::equal(std::begin(tLhs), std::begin(tLhs), std::begin(aRhs), [](const auto& lhs, const auto& rhs) {
        return std::any_cast<ValueType>(lhs) == rhs;
      });
    }
  }

  // Template specialization for std::list<...>
  template <typename T>
  typename std::enable_if<is_specialization<T, std::list>::value, bool>::type
  operator==(const T& aRhs) const {
    using ValueType = typename T::value_type;
    T tLhs = get<T>();
    if (tLhs.size() != aRhs.size())
      return false;
    else {
      return std::equal(std::begin(tLhs), std::begin(tLhs), std::begin(aRhs), [](const auto& lhs, const auto& rhs) {
        return std::any_cast<ValueType>(lhs) == rhs;
      });
    }
  }

  private:
    // If an argument starts with "-" or "--", then it's optional
    static bool is_optional(const std::string& aName) {
      return (starts_with(aName, "--") || starts_with(aName, "-"));
    }

    // Getter for template types other than std::vector and std::list
    template <typename T>
    typename std::enable_if<!is_specialization<T, std::vector>::value &&
                            !is_specialization<T, std::list>::value, T>::type
    get() const {
      if (mValues.empty()) {
        if (mDefaultValue.has_value()) {
          return std::any_cast<T>(mDefaultValue);
        }
        else
          return T();
      }
      else {
        if (!mRawValues.empty())
          return std::any_cast<T>(mValues[0]);
        else {
          if (mDefaultValue.has_value())
            return std::any_cast<T>(mDefaultValue);
          else
            return T();
        }
      }
    }

    // Getter for std::vector. Here T = std::vector<...>
    template <typename T>
    typename std::enable_if<is_specialization<T, std::vector>::value, T>::type
    get() const {
      using ValueType = typename T::value_type;
      T tResult;
      if (mValues.empty()) {
        if (mDefaultValue.has_value()) {
          T tDefaultValues = std::any_cast<T>(mDefaultValue);
          std::transform(std::begin(tDefaultValues),  std::end(tDefaultValues),
                         std::back_inserter(tResult), std::any_cast<ValueType>);
          return tResult;
        }
        else
          return T();
      }
      else {
        if (!mRawValues.empty()) {
          for (const auto& mValue : mValues) {
            tResult.emplace_back(std::any_cast<ValueType>(mValue));
          }
          return tResult;
        }
        else {
          if (mDefaultValue.has_value()) {
            auto tDefaultValues = std::any_cast<std::vector<T>>(mDefaultValue);
            std::transform(std::begin(tDefaultValues),  std::end(tDefaultValues),
                           std::back_inserter(tResult), std::any_cast<ValueType>);
            return tResult;
          }
          else
            return T();
        }
      }
    }

    // Getter for std::list. Here T = std::list<...>
    template <typename T>
    typename std::enable_if<is_specialization<T, std::list>::value, T>::type
    get() const {
      using ValueType = typename T::value_type;
      T tResult;
      if (mValues.empty()) {
        if (mDefaultValue.has_value()) {
          T tDefaultValues = std::any_cast<T>(mDefaultValue);
          std::transform(std::begin(tDefaultValues),  std::end(tDefaultValues),
                         std::back_inserter(tResult), std::any_cast<ValueType>);
          return tResult;
        }
        else
          return T();
      }
      else {
        if (!mRawValues.empty()) {
          for (const auto& mValue : mValues) {
            tResult.emplace_back(std::any_cast<ValueType>(mValue));
          }
          return tResult;
        }
        else {
          if (mDefaultValue.has_value()) {
            auto tDefaultValues = std::any_cast<std::list<T>>(mDefaultValue);
            std::transform(std::begin(tDefaultValues),  std::end(tDefaultValues),
                           std::back_inserter(tResult), std::any_cast<ValueType>);
            return tResult;
          }
          else
            return T();
        }
      }
    }

    std::vector<std::string> mNames;
    std::string mUsedName;
    std::string mHelp;
    std::any mDefaultValue;
    std::any mImplicitValue;
    std::function<std::any(const std::string&)> mAction = [](const std::string& aValue) { return aValue; };
    std::vector<std::any> mValues;
    std::vector<std::string> mRawValues;
    size_t mNumArgs = 1;
    bool mIsOptional = false;
    bool mIsUsed = false; // relevant for optional arguments. True if used by user

  public:
    static constexpr auto mHelpOption = "-h";
    static constexpr auto mHelpOptionLong = "--help";
};

class ArgumentParser {
  public:
    explicit ArgumentParser(std::string aProgramName = {}) :
      mProgramName(std::move(aProgramName))
    {
      add_argument(Argument::mHelpOption, Argument::mHelpOptionLong)
        .help("show this help message and exit")
        .nargs(0)
        .default_value(false)
        .implicit_value(true);
    }

    // Parameter packing
    // Call add_argument with variadic number of string arguments
    template<typename... Targs>
    Argument& add_argument(Targs... Fargs) {
      std::shared_ptr<Argument> tArgument = std::make_shared<Argument>(std::move(Fargs)...);

      if (tArgument->mIsOptional)
        mOptionalArguments.emplace_back(tArgument);
      else
        mPositionalArguments.emplace_back(tArgument);

      for (const auto& mName : tArgument->mNames) {
        mArgumentMap.insert_or_assign(mName, tArgument);
      }
      return *tArgument;
    }

    // Parameter packed add_parents method
    // Accepts a variadic number of ArgumentParser objects
    template<typename... Targs>
    void add_parents(Targs... Fargs) {
      const auto tNewParentParsers = {Fargs...};
      for (const auto& tParentParser : tNewParentParsers) {
        const auto& tPositionalArguments = tParentParser.mPositionalArguments;
        std::copy(std::begin(tPositionalArguments), std::end(tPositionalArguments), std::back_inserter(mPositionalArguments));

        const auto& tOptionalArguments = tParentParser.mOptionalArguments;
        std::copy(std::begin(tOptionalArguments), std::end(tOptionalArguments), std::back_inserter(mOptionalArguments));

        const auto& tArgumentMap = tParentParser.mArgumentMap;
        for (const auto&[tKey, tValue] : tArgumentMap) {
          mArgumentMap.insert_or_assign(tKey, tValue);
        }
      }
      std::move(std::begin(tNewParentParsers), std::end(tNewParentParsers), std::back_inserter(mParentParsers));
    }

    /* Call parse_args_internal - which does all the work
     * Then, validate the parsed arguments
     * This variant is used mainly for testing
     * @throws std::runtime_error in case of any invalid argument
     */
    void parse_args(const std::vector<std::string>& aArguments) {
      parse_args_internal(aArguments);
      parse_args_validate();
    }

    /* Main entry point for parsing command-line arguments using this ArgumentParser
     * @throws std::runtime_error in case of any invalid argument
     */
    void parse_args(int argc, char * argv[]) {
      parse_args_internal(argc, argv);
      parse_args_validate();
    }

    // Getter enabled for all template types other than std::vector and std::list
    template <typename T = std::string>
    T get(const std::string& aArgumentName) {
      auto tIterator = mArgumentMap.find(aArgumentName);
      if (tIterator != mArgumentMap.end()) {
        return tIterator->second->get<T>();
      }
      return T();
    }

    // Indexing operator. Return a reference to an Argument object
    // Used in conjuction with Argument.operator== e.g., parser["foo"] == true
    Argument& operator[](const std::string& aArgumentName) {
      auto tIterator = mArgumentMap.find(aArgumentName);
      if (tIterator != mArgumentMap.end()) {
        return *(tIterator->second);
      }
      else {
        throw std::runtime_error("Argument " + aArgumentName + " not found");
      }
    }

    // Printing the one and only help message
    // I've stuck with a simple message format, nothing fancy.
    // TODO: support user-defined help and usage messages for the ArgumentParser
    std::string print_help() {
      std::stringstream stream;
      stream << "Usage: " << mProgramName << " [options]";
      size_t tLongestArgumentLength = get_length_of_longest_argument();

      for (size_t i = 0; i < mPositionalArguments.size(); i++) {
        auto tNames = mPositionalArguments[i]->mNames;
        stream << (i == 0 ? " " : "") << tNames[0] << " ";
      }
      stream << "\n\n";

      if (!mPositionalArguments.empty())
        stream << "Positional arguments:\n";
      for (const auto& mPositionalArgument : mPositionalArguments) {
        size_t tCurrentLength = 0;
        auto tNames = mPositionalArgument->mNames;
        for (size_t j = 0; j < tNames.size() - 1; j++) {
          auto tCurrentName = tNames[j];
          stream << tCurrentName;
          stream << ", ";
          tCurrentLength += tCurrentName.length() + 2;
        }
        stream << tNames[tNames.size() - 1];
        tCurrentLength += tNames[tNames.size() - 1].length();
        if (tCurrentLength < tLongestArgumentLength)
          stream << std::string((tLongestArgumentLength - tCurrentLength) + 2, ' ');
        else if (tCurrentLength == tLongestArgumentLength)
          stream << std::string(2, ' ');
        else
          stream << std::string((tCurrentLength - tLongestArgumentLength) + 2, ' ');

        stream << mPositionalArgument->mHelp << "\n";
      }

      if (!mOptionalArguments.empty() && !mPositionalArguments.empty())
        stream << "\nOptional arguments:\n";
      else if (!mOptionalArguments.empty())
        stream << "Optional arguments:\n";
      for (const auto & mOptionalArgument : mOptionalArguments) {
        size_t tCurrentLength = 0;
        auto tNames = mOptionalArgument->mNames;
        std::sort(tNames.begin(), tNames.end(),
          [](const std::string& lhs, const std::string& rhs) {
            return lhs.size() == rhs.size() ? lhs < rhs : lhs.size() < rhs.size();
        });
        for (size_t j = 0; j < tNames.size() - 1; j++) {
          auto tCurrentName = tNames[j];
          stream << tCurrentName;
          stream << ", ";
          tCurrentLength += tCurrentName.length() + 2;
        }
        stream << tNames[tNames.size() - 1];
        tCurrentLength += tNames[tNames.size() - 1].length();
        if (tCurrentLength < tLongestArgumentLength)
          stream << std::string((tLongestArgumentLength - tCurrentLength) + 2, ' ');
        else if (tCurrentLength == tLongestArgumentLength)
          stream << std::string(2, ' ');
        else
          stream << std::string((tCurrentLength - tLongestArgumentLength) + 2, ' ');

        stream << mOptionalArgument->mHelp << "\n";
      }

      std::cout << stream.str();
      return stream.str();
    }

  private:
    // If the argument was defined by the user and can be found in mArgumentMap, then it's valid
    bool is_valid_argument(const std::string& aName) {
      auto tIterator = mArgumentMap.find(aName);
      return (tIterator != mArgumentMap.end());
    }

    /*
     * @throws std::runtime_error in case of any invalid argument
     */
    void parse_args_internal(const std::vector<std::string>& aArguments) {
      std::vector<char*> argv;
      for (const auto& arg : aArguments)
        argv.emplace_back(const_cast<char*>(arg.data()));
      argv.emplace_back(nullptr);
      return parse_args_internal(int(argv.size()) - 1, argv.data());
    }

    /*
     * @throws std::runtime_error in case of any invalid argument
     */
    void parse_args_internal(int argc, char * argv[]) {
      if (mProgramName.empty() && argc > 0)
        mProgramName = argv[0];
      for (int i = 1; i < argc; i++) {
        auto tCurrentArgument = std::string(argv[i]);
        if (tCurrentArgument == Argument::mHelpOption || tCurrentArgument == Argument::mHelpOptionLong) {
          throw std::runtime_error("help called");
        }
        auto tIterator = mArgumentMap.find(argv[i]);
        if (tIterator != mArgumentMap.end()) {
          // Start parsing optional argument
          auto tArgument = tIterator->second;
          tArgument->mUsedName = tCurrentArgument;
          tArgument->mIsUsed = true;
          auto tCount = tArgument->mNumArgs;

          // Check to see if implicit value should be used
          // Two cases to handle here:
          // (1) User has explicitly programmed nargs to be 0
          // (2) User has provided an implicit value, which also sets nargs to 0
          if (tCount == 0) {
            // Use implicit value for this optional argument
            tArgument->mValues.emplace_back(tArgument->mImplicitValue);
            tArgument->mRawValues.emplace_back();
            tCount = 0;
          }
          while (tCount > 0) {
            i = i + 1;
            if (i < argc) {
              tArgument->mUsedName = tCurrentArgument;
              tArgument->mRawValues.emplace_back(argv[i]);
              if (tArgument->mAction != nullptr)
                tArgument->mValues.emplace_back(tArgument->mAction(argv[i]));
              else {
                if (tArgument->mDefaultValue.has_value())
                  tArgument->mValues.emplace_back(tArgument->mDefaultValue);
                else
                  tArgument->mValues.emplace_back(std::string(argv[i]));
              }
            }
            tCount -= 1;
          }
        }
        else {
          if (Argument::is_optional(argv[i])) {
            // This is possibly a compound optional argument
            // Example: We have three optional arguments -a, -u and -x
            // The user provides ./main -aux ...
            // Here -aux is a compound optional argument
            std::string tCompoundArgument = std::string(argv[i]);
            if (tCompoundArgument.size() > 1 && tCompoundArgument[0] == '-' && tCompoundArgument[1] != '-') {
              for (size_t j = 1; j < tCompoundArgument.size(); j++) {
                std::string tArgument(1, tCompoundArgument[j]);
                size_t tNumArgs = 0;
                tIterator = mArgumentMap.find("-" + tArgument);
                if (tIterator != mArgumentMap.end()) {
                  auto tArgumentObject = tIterator->second;
                  tNumArgs = tArgumentObject->mNumArgs;
                  std::vector<std::string> tArgumentsForRecursiveParsing = {"", "-" + tArgument};
                  while (tNumArgs > 0 && i < argc) {
                    i += 1;
                    if (i < argc) {
                      tArgumentsForRecursiveParsing.emplace_back(argv[i]);
                      tNumArgs -= 1;
                    }
                  }
                  parse_args_internal(tArgumentsForRecursiveParsing);
                }
                else {
                  if (!tArgument.empty() && tArgument[0] == '-')
                    std::cout << "warning: unrecognized optional argument " << tArgument
                              << std::endl;
                  else
                    std::cout << "warning: unrecognized optional argument -" << tArgument
                              << std::endl;
                }
              }
            }
            else {
              std::cout << "warning: unrecognized optional argument " << tCompoundArgument << std::endl;
            }
          }
          else {
            // This is a positional argument.
            // Parse and save into mPositionalArguments vector
            if (mNextPositionalArgument >= mPositionalArguments.size()) {
              std::stringstream stream;
              stream << "error: unexpected positional argument " << argv[i] << std::endl;
              throw std::runtime_error(stream.str());
            }
            auto tArgument = mPositionalArguments[mNextPositionalArgument];
            auto tCount = tArgument->mNumArgs - tArgument->mRawValues.size();
            while (tCount > 0) {
              tIterator = mArgumentMap.find(argv[i]);
              if (tIterator != mArgumentMap.end() || Argument::is_optional(argv[i])) {
                i = i - 1;
                break;
              }
              if (i < argc) {
                tArgument->mUsedName = tCurrentArgument;
                tArgument->mRawValues.emplace_back(argv[i]);
                if (tArgument->mAction != nullptr)
                  tArgument->mValues.emplace_back(tArgument->mAction(argv[i]));
                else {
                  if (tArgument->mDefaultValue.has_value())
                    tArgument->mValues.emplace_back(tArgument->mDefaultValue);
                  else
                    tArgument->mValues.emplace_back(std::string(argv[i]));
                }
              }
              tCount -= 1;
              if (tCount > 0) i += 1;
            }
            if (tCount == 0)
              mNextPositionalArgument += 1;
          }
        }
      }
    }

    /*
     * @throws std::runtime_error in case of any invalid argument
     */
    void parse_args_validate() {
      try {
        // Check if all positional arguments are parsed
        std::for_each(std::begin(mPositionalArguments),
                      std::end(mPositionalArguments),
                      std::mem_fn(&Argument::validate));
        // Check if all user-provided optional argument values are parsed correctly
        std::for_each(std::begin(mOptionalArguments),
                      std::end(mOptionalArguments),
                      std::mem_fn(&Argument::validate));
      } catch (const std::runtime_error& err) {
        throw err;
      }
    }

    // Used by print_help.
    size_t get_length_of_longest_argument(const std::vector<std::shared_ptr<Argument>>& aArguments) {
      if (aArguments.empty())
        return 0;
      std::vector<size_t> argumentLengths(aArguments.size());
      std::transform(std::begin(aArguments), std::end(aArguments), std::begin(argumentLengths), [](const auto& arg) {
        const auto& names = arg->mNames;
        auto maxLength = std::accumulate(std::begin(names), std::end(names), 0, [](const auto& sum, const auto& s) {
          return sum + s.size() + 2; // +2 for ", "
        });
        return maxLength - 2; // -2 since the last one doesn't need ", "
      });
      return *std::max_element(std::begin(argumentLengths), std::end(argumentLengths));
    }

    // Used by print_help.
    size_t get_length_of_longest_argument() {
      const auto positionalArgMaxSize = get_length_of_longest_argument(mPositionalArguments);
      const auto optionalArgMaxSize = get_length_of_longest_argument(mOptionalArguments);

      return std::max(positionalArgMaxSize, optionalArgMaxSize);
    }

    std::string mProgramName;
    std::vector<ArgumentParser> mParentParsers;
    std::vector<std::shared_ptr<Argument>> mPositionalArguments;
    std::vector<std::shared_ptr<Argument>> mOptionalArguments;
    size_t mNextPositionalArgument = 0;
    std::map<std::string, std::shared_ptr<Argument>> mArgumentMap;
};

#define PARSE_ARGS(parser, argc, argv) \
try { \
  parser.parse_args(argc, argv); \
} catch (const std::runtime_error& err) { \
  std::cout << err.what() << std::endl; \
  parser.print_help(); \
  exit(0); \
}

}
