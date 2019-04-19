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

namespace argparse {

// Some utility structs to check template specialization
template<typename Test, template<typename...> class Ref>
struct is_specialization : std::false_type {};

template<template<typename...> class Ref, typename... Args>
struct is_specialization<Ref<Args...>, Ref> : std::true_type {};

// Upsert into std::map
template <class KeyType, class ElementType>
bool upsert(std::map<KeyType, ElementType>& aMap, KeyType const& aKey, ElementType const& aNewValue) {
  typedef typename std::map<KeyType, ElementType>::iterator Iterator;
  typedef typename std::pair<Iterator, bool> Result;
  Result tResult = aMap.insert(typename std::map<KeyType, ElementType>::value_type(aKey, aNewValue));
  if (!tResult.second) {
    if (!(tResult.first->second == aNewValue)) {
      tResult.first->second = aNewValue;
      return true;
    }
    else
      return false; // it was the same
  }
  else
    return true;  // changed cause not existing
}

// Check if string (haystack) starts with a substring (needle)
bool starts_with(const std::string& haystack, const std::string& needle) {
  return needle.length() <= haystack.length()
    && std::equal(needle.begin(), needle.end(), haystack.begin());
};

// Get value at index from std::list
template <typename T>
T get_from_list(const std::list<T>& aList, size_t aIndex) {
  if (aList.size() > aIndex) {
    auto tIterator = aList.begin();
    std::advance(tIterator, aIndex);
    return *tIterator;
  }
  return T();
}

class Argument {
  friend class ArgumentParser;
public:
  Argument() :
    mNames({}),
    mHelp(""),
    mAction([](const std::string& aValue) { return aValue; }),
    mValues({}),
    mRawValues({}),
    mNumArgs(1),
    mIsOptional(false),
    mIsUsed(false) {}

  Argument& help(const std::string& aHelp) {
    mHelp = aHelp;
    return *this;
  }

  Argument& default_value(std::any aDefaultValue) {
    mDefaultValue = aDefaultValue;
    return *this;
  }

  Argument& implicit_value(std::any aImplicitValue) {
    mImplicitValue = aImplicitValue;
    mNumArgs = 0;
    return *this;
  }

  Argument& action(std::function<std::any(const std::string&)> aAction) {
    mAction = aAction;
    return *this;
  }

  Argument& nargs(size_t aNumArgs) {
    mNumArgs = aNumArgs;
    return *this;
  }

  template <typename T>
  bool operator!=(const T& aRhs) const {
    return !(*this == aRhs);
  }

  // Entry point for template types other than std::vector and std::list
  template <typename T>
  typename std::enable_if<is_specialization<T, std::vector>::value == false &&
                          is_specialization<T, std::list>::value == false, bool>::type
    operator==(const T& aRhs) const {
    return get<T>() == aRhs;
  }

  // Template specialization for std::vector<...>
  template <typename T>
  typename std::enable_if<is_specialization<T, std::vector>::value, bool>::type
  operator==(const T& aRhs) const {
    T tLhs = get_vector<T>();
    if (tLhs.size() != aRhs.size())
      return false;
    else {
      for (size_t i = 0; i < tLhs.size(); i++) {
        auto tValueAtIndex = std::any_cast<typename T::value_type>(tLhs[i]);
        if (tValueAtIndex != aRhs[i])
          return false;
      }
      return true;
    }
  }

  // Template specialization for std::list<...>
  template <typename T>
  typename std::enable_if<is_specialization<T, std::list>::value, bool>::type
    operator==(const T& aRhs) const {
    T tLhs = get_list<T>();
    if (tLhs.size() != aRhs.size())
      return false;
    else {
      for (size_t i = 0; i < tLhs.size(); i++) {
        auto tValueAtIndex = std::any_cast<typename T::value_type>(get_from_list(tLhs, i));
        if (tValueAtIndex != get_from_list(aRhs, i))
          return false;
      }
      return true;
    }
  }

  private:

    // Getter for template types other than std::vector and std::list
    template <typename T>
    T get() const {
      if (mValues.size() == 0) {
        if (mDefaultValue.has_value()) {
          return std::any_cast<T>(mDefaultValue);
        }
        else
          return T();
      }
      else {
        if (mRawValues.size() > 0)
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
    T get_vector() const {
      T tResult;
      if (mValues.size() == 0) {
        if (mDefaultValue.has_value()) {
          T tDefaultValues = std::any_cast<T>(mDefaultValue);
          for (size_t i = 0; i < tDefaultValues.size(); i++) {
            tResult.push_back(std::any_cast<typename T::value_type>(tDefaultValues[i]));
          }
          return tResult;
        }
        else
          return T();
      }
      else {
        if (mRawValues.size() > 0) {
          for (size_t i = 0; i < mValues.size(); i++) {
            tResult.push_back(std::any_cast<typename T::value_type>(mValues[i]));
          }
          return tResult;
        }
        else {
          if (mDefaultValue.has_value()) {
            std::vector<T> tDefaultValues = std::any_cast<std::vector<T>>(mDefaultValue);
            for (size_t i = 0; i < tDefaultValues.size(); i++) {
              tResult.push_back(std::any_cast<typename T::value_type>(tDefaultValues[i]));
            }
            return tResult;
          }
          else
            return T();
        }
      }
    }

    // Getter for std::list. Here T = std::list<...>
    template <typename T>
    T get_list() const {
      T tResult;
      if (mValues.size() == 0) {
        if (mDefaultValue.has_value()) {
          T tDefaultValues = std::any_cast<T>(mDefaultValue);
          for (size_t i = 0; i < tDefaultValues.size(); i++) {
            tResult.push_back(std::any_cast<typename T::value_type>(get_from_list(tDefaultValues, i)));
          }
          return tResult;
        }
        else
          return T();
      }
      else {
        if (mRawValues.size() > 0) {
          for (size_t i = 0; i < mValues.size(); i++) {
            tResult.push_back(std::any_cast<typename T::value_type>(mValues[i]));
          }
          return tResult;
        }
        else {
          if (mDefaultValue.has_value()) {
            std::list<T> tDefaultValues = std::any_cast<std::list<T>>(mDefaultValue);
            for (size_t i = 0; i < tDefaultValues.size(); i++) {
              tResult.push_back(std::any_cast<typename T::value_type>(get_from_list(tDefaultValues, i)));
            }
            return tResult;
          }
          else
            return T();
        }
      }
    }

    std::vector<std::string> mNames;
    std::string mHelp;
    std::any mDefaultValue;
    std::any mImplicitValue;
    std::function<std::any(const std::string&)> mAction;
    std::vector<std::any> mValues;
    std::vector<std::string> mRawValues;
    size_t mNumArgs;
    bool mIsOptional;
    bool mIsUsed; // relevant for optional arguments. True if used by user
};

class ArgumentParser {
  public:
    ArgumentParser(const std::string& aProgramName = "") :
      mProgramName(aProgramName),
      mNextPositionalArgument(0) {
      std::shared_ptr<Argument> tArgument = std::make_shared<Argument>();
      tArgument->mNames = { "-h", "--help" };
      tArgument->mHelp = "show this help message and exit";
      tArgument->mNumArgs = 0;
      tArgument->mDefaultValue = false;
      tArgument->mImplicitValue = true;
      mOptionalArguments.push_back(tArgument);
      upsert(mArgumentMap, std::string("-h"), tArgument);
      upsert(mArgumentMap, std::string("--help"), tArgument);
    }

    // Parameter packing
    // Call add_argument with variadic number of string arguments
    // TODO: enforce T to be std::string
    template<typename T, typename... Targs>
    Argument& add_argument(T value, Targs... Fargs) {
      std::shared_ptr<Argument> tArgument = std::make_shared<Argument>();
      tArgument->mNames.push_back(value);
      add_argument_internal(tArgument, Fargs...);

      for (auto& mName : tArgument->mNames) {
        if (is_optional(mName))
          tArgument->mIsOptional = true;
      }

      if (!tArgument->mIsOptional)
        mPositionalArguments.push_back(tArgument);
      else
        mOptionalArguments.push_back(tArgument);

      for (auto& mName : tArgument->mNames) { 
        upsert(mArgumentMap, mName, tArgument);
      }
      return *tArgument;
    }

    // Base case for add_parents parameter packing
    void add_parents() {
      for (size_t i = 0; i < mParentParsers.size(); i++) {
        auto tParentParser = mParentParsers[i];
        auto tPositionalArguments = tParentParser.mPositionalArguments;
        for (auto& tArgument : tPositionalArguments) {
          mPositionalArguments.push_back(tArgument);
        }
        auto tOptionalArguments = tParentParser.mOptionalArguments;
        for (auto& tArgument : tOptionalArguments) {
          mOptionalArguments.push_back(tArgument);
        }
        auto tArgumentMap = tParentParser.mArgumentMap;
        for (auto&[tKey, tValue] : tArgumentMap) {
          upsert(mArgumentMap, tKey, tValue);
        }
      }
    }

    // Parameter packed add_parents method
    // Accepts a variadic number of ArgumentParser objects
    template<typename T, typename... Targs>
    void add_parents(T aArgumentParser, Targs... Fargs) {
      mParentParsers.push_back(aArgumentParser);
      add_parents(Fargs...);
    }

    // Call parse_args_internal - which does all the work
    // Then, validate the parsed arguments
    // This variant is used mainly for testing
    void parse_args(const std::vector<std::string>& aArguments) {
      parse_args_internal(aArguments);
      parse_args_validate();
    }

    // Main entry point for parsing command-line arguments using this ArgumentParser
    void parse_args(int argc, char * argv[]) {
      parse_args_internal(argc, argv);
      parse_args_validate();
    }

    // Getter enabled for all template types other than std::vector and std::list
    template <typename T = std::string>
    typename std::enable_if<is_specialization<T, std::vector>::value == false && 
                            is_specialization<T, std::list>::value == false, T>::type
    get(const char * aArgumentName) {
      std::map<std::string, std::shared_ptr<Argument>>::iterator tIterator = mArgumentMap.find(aArgumentName);
      if (tIterator != mArgumentMap.end()) {
        return tIterator->second->get<T>();
      }
      return T();
    }

    // Getter enabled for std::vector
    template <typename T>
    typename std::enable_if<is_specialization<T, std::vector>::value, T>::type
    get(const char * aArgumentName) {
      std::map<std::string, std::shared_ptr<Argument>>::iterator tIterator = mArgumentMap.find(aArgumentName);
      if (tIterator != mArgumentMap.end()) {
        return tIterator->second->get_vector<T>();
      }
      return T();
    }

    // Getter enabled for std::list
    template <typename T>
    typename std::enable_if<is_specialization<T, std::list>::value, T>::type
      get(const char * aArgumentName) {
      std::map<std::string, std::shared_ptr<Argument>>::iterator tIterator = mArgumentMap.find(aArgumentName);
      if (tIterator != mArgumentMap.end()) {
        return tIterator->second->get_list<T>();
      }
      return T();
    }

    // Indexing operator. Return a reference to an Argument object
    // Used in conjuction with Argument.operator== e.g., parser["foo"] == true
    Argument& operator[](const std::string& aArgumentName) {
      std::map<std::string, std::shared_ptr<Argument>>::iterator tIterator = mArgumentMap.find(aArgumentName);
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

      if (mPositionalArguments.size() > 0)
        stream << "Positional arguments:\n";
      for (size_t i = 0; i < mPositionalArguments.size(); i++) {
        size_t tCurrentLength = 0;
        auto tNames = mPositionalArguments[i]->mNames;
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
        
        stream << mPositionalArguments[i]->mHelp << "\n";
      }

      if (mOptionalArguments.size() > 0 && mPositionalArguments.size() > 0)
        stream << "\nOptional arguments:\n";
      else if (mOptionalArguments.size() > 0)
        stream << "Optional arguments:\n";
      for (size_t i = 0; i < mOptionalArguments.size(); i++) {
        size_t tCurrentLength = 0;
        auto tNames = mOptionalArguments[i]->mNames;
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

        stream << mOptionalArguments[i]->mHelp << "\n";
      }

      std::cout << stream.str();
      return stream.str();
    }

  private:
    Argument& add_argument_internal(std::shared_ptr<Argument> aArgument) {
      return *aArgument;
    }

    template<typename T, typename... Targs>
    Argument& add_argument_internal(std::shared_ptr<Argument> aArgument, T aArgumentName, Targs... Fargs) {
      aArgument->mNames.push_back(aArgumentName);
      add_argument_internal(aArgument, Fargs...);
      return *aArgument;
    }

    // If an argument starts with "-" or "--", then it's optional
    bool is_optional(const std::string& aName) {
      return (starts_with(aName, "--") || starts_with(aName, "-"));
    }

    // If the argument was defined by the user and can be found in mArgumentMap, then it's valid
    bool is_valid_argument(const std::string& aName) {
      std::map<std::string, std::shared_ptr<Argument>>::iterator tIterator = mArgumentMap.find(aName);
      return (tIterator != mArgumentMap.end());
    }

    void parse_args_internal(const std::vector<std::string>& aArguments) {
      std::vector<char*> argv;
      for (const auto& arg : aArguments)
        argv.push_back((char*)arg.data());
      argv.push_back(nullptr);
      return parse_args_internal(argv.size() - 1, argv.data());
    }

    void parse_args_internal(int argc, char * argv[]) {
      if (mProgramName == "" && argc > 0)
        mProgramName = argv[0];
      for (int i = 1; i < argc; i++) {
        auto tCurrentArgument = std::string(argv[i]);
        if (tCurrentArgument == "-h" || tCurrentArgument == "--help") {
          print_help();
          exit(0);
        }
        std::map<std::string, std::shared_ptr<Argument>>::iterator tIterator = mArgumentMap.find(argv[i]);
        if (tIterator != mArgumentMap.end()) {
          // Start parsing optional argument
          auto tArgument = tIterator->second;
          tArgument->mIsUsed = true;
          auto tCount = tArgument->mNumArgs;

          // Check to see if implicit value should be used
          // Two cases to handle here:
          // (1) User has explicitly programmed nargs to be 0
          // (2) User has provided an implicit value, which also sets nargs to 0
          if (tCount == 0) {
            // Use implicit value for this optional argument
            tArgument->mValues.push_back(tArgument->mImplicitValue);
            tArgument->mRawValues.push_back("");
            tCount = 0;
          }
          while (tCount > 0) {
            i = i + 1;
            if (i < argc) {
              tArgument->mRawValues.push_back(argv[i]);
              if (tArgument->mAction != nullptr)
                tArgument->mValues.push_back(tArgument->mAction(argv[i]));
              else {
                if (tArgument->mDefaultValue.has_value())
                  tArgument->mValues.push_back(tArgument->mDefaultValue);
                else
                  tArgument->mValues.push_back(std::string(argv[i]));
              }
            }
            tCount -= 1;
          }
        }
        else {
          if (is_optional(argv[i])) {
            // This is possibly a compound optional argument
            // Example: We have three optional arguments -a, -u and -x
            // The user provides ./main -aux ...
            // Here -aux is a compound optional argument
            std::string tCompoundArgument = std::string(argv[i]);
            if (tCompoundArgument.size() > 1 && tCompoundArgument[0] == '-' && tCompoundArgument[1] != '-') {
              for (size_t j = 1; j < tCompoundArgument.size(); j++) {
                std::string tArgument(1, tCompoundArgument[j]);
                size_t tNumArgs = 0;
                std::map<std::string, std::shared_ptr<Argument>>::iterator tIterator = mArgumentMap.find("-" + tArgument);
                if (tIterator != mArgumentMap.end()) {
                  auto tArgumentObject = tIterator->second;
                  tNumArgs = tArgumentObject->mNumArgs;
                }
                std::vector<std::string> tArgumentsForRecursiveParsing = { "", "-" + tArgument };
                while (tNumArgs > 0 && i < argc) {
                  i += 1;
                  if (i < argc) {
                    tArgumentsForRecursiveParsing.push_back(argv[i]);
                    tNumArgs -= 1;
                  }
                }
                parse_args_internal(tArgumentsForRecursiveParsing);
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
              std::cout << "error: unexpected positional argument " << argv[i] << std::endl;
              print_help();
              exit(0);
            }
            auto tArgument = mPositionalArguments[mNextPositionalArgument];
            auto tCount = tArgument->mNumArgs - tArgument->mRawValues.size();
            while (tCount > 0) {
              std::map<std::string, std::shared_ptr<Argument>>::iterator tIterator = mArgumentMap.find(argv[i]);
              if (tIterator != mArgumentMap.end() || is_optional(argv[i])) {
                i = i - 1;
                break;
              }
              if (i < argc) {
                tArgument->mRawValues.push_back(argv[i]);
                if (tArgument->mAction != nullptr)
                  tArgument->mValues.push_back(tArgument->mAction(argv[i]));
                else {
                  if (tArgument->mDefaultValue.has_value())
                    tArgument->mValues.push_back(tArgument->mDefaultValue);
                  else
                    tArgument->mValues.push_back(std::string(argv[i]));
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

    void parse_args_validate() {
      // Check if all positional arguments are parsed
      for (size_t i = 0; i < mPositionalArguments.size(); i++) {
        auto tArgument = mPositionalArguments[i];
        if (tArgument->mValues.size() != tArgument->mNumArgs) {
          std::cout << "error: " << tArgument->mNames[0] << ": expected "
            << tArgument->mNumArgs << (tArgument->mNumArgs == 1 ? "argument. " : " arguments. ")
            << tArgument->mValues.size() << " provided.\n" << std::endl;
          print_help();
          exit(0);
        }
      }

      // Check if all user-provided optional argument values are parsed correctly
      for (size_t i = 0; i < mOptionalArguments.size(); i++) {
        auto tArgument = mOptionalArguments[i];
        if (tArgument->mIsUsed && tArgument->mNumArgs > 0) {
          if (tArgument->mValues.size() != tArgument->mNumArgs) {
            // All cool if there's a default value to return
            // If no default value, then there's a problem
            if (!tArgument->mDefaultValue.has_value()) {
              std::cout << "error: " << tArgument->mNames[0] << ": expected "
                << tArgument->mNumArgs << (tArgument->mNumArgs == 1 ? "argument. " : " arguments. ")
                << tArgument->mValues.size() << " provided.\n" << std::endl;
              print_help();
              exit(0);
            }
          }
        }
        else {
          // TODO: check if an implicit value was programmed for this argument
        }
      }
    }

    // Used by print_help. 
    size_t get_length_of_longest_argument() {
      size_t tResult = 0;
      for (size_t i = 0; i < mPositionalArguments.size(); i++) {
        size_t tCurrentArgumentLength = 0;
        auto tNames = mPositionalArguments[i]->mNames;
        for (size_t j = 0; j < tNames.size() - 1; j++) {
          auto tNameLength = tNames[j].length();
          tCurrentArgumentLength += tNameLength + 2; // +2 for ", "
        }
        tCurrentArgumentLength += tNames[tNames.size() - 1].length();
        if (tCurrentArgumentLength > tResult)
          tResult = tCurrentArgumentLength;
      }

      for (size_t i = 0; i < mOptionalArguments.size(); i++) {
        size_t tCurrentArgumentLength = 0;
        auto tNames = mOptionalArguments[i]->mNames;
        for (size_t j = 0; j < tNames.size() - 1; j++) {
          auto tNameLength = tNames[j].length();
          tCurrentArgumentLength += tNameLength + 2; // +2 for ", "
        }
        tCurrentArgumentLength += tNames[tNames.size() - 1].length();
        if (tCurrentArgumentLength > tResult)
          tResult = tCurrentArgumentLength;
      }
      return tResult;
    }

    std::string mProgramName;
    std::vector<ArgumentParser> mParentParsers;
    std::vector<std::shared_ptr<Argument>> mPositionalArguments;
    std::vector<std::shared_ptr<Argument>> mOptionalArguments;
    size_t mNextPositionalArgument;
    std::map<std::string, std::shared_ptr<Argument>> mArgumentMap;
};

}