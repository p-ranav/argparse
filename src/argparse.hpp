#pragma once
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

template<typename Test, template<typename...> class Ref>
struct is_specialization : std::false_type {};

template<template<typename...> class Ref, typename... Args>
struct is_specialization<Ref<Args...>, Ref> : std::true_type {};

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

bool starts_with(const std::string& haystack, const std::string& needle) {
  return needle.length() <= haystack.length()
    && std::equal(needle.begin(), needle.end(), haystack.begin());
};

template <typename T>
T get_from_list(const std::list<T>& aList, size_t aIndex) {
  if (aList.size() > aIndex) {
    auto tIterator = aList.begin();
    std::advance(tIterator, aIndex);
    return *tIterator;
  }
  return T();
}

template <typename T>
T get_from_list(const std::vector<T>& aList, size_t aIndex) {
  return aList[aIndex];
}

struct Argument {
  std::vector<std::string> mNames;
  std::string mHelp;
  std::any mDefaultValue;
  std::any mImplicitValue;
  std::function<std::any(const std::string&)> mAction;
  std::vector<std::any> mValues;
  std::vector<std::string> mRawValues;
  size_t mNumArgs;
  bool mIsOptional;

  Argument() :
    mNames({}),
    mHelp(""),
    mAction([](const std::string& aValue) { return aValue; }),
    mValues({}),
    mRawValues({}),
    mNumArgs(1),
    mIsOptional(false) {}

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

  template <typename T>
  bool operator!=(const T& aRhs) const {
    return !(*this == aRhs);
  }

  template <typename T>
  typename std::enable_if<is_specialization<T, std::vector>::value == false &&
                          is_specialization<T, std::list>::value == false, bool>::type
    operator==(const T& aRhs) const {
    return get<T>() == aRhs;
  }

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

};

class ArgumentParser {
  public:
    ArgumentParser(const std::string& aProgramName = "") :
      mProgramName(aProgramName),
      mNextPositionalArgument(0) {}

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

    void parse_args(const std::vector<std::string>& aArguments) {
      std::vector<char*> argv;
      for (const auto& arg : aArguments)
        argv.push_back((char*)arg.data());
      argv.push_back(nullptr);
      return parse_args(argv.size() - 1, argv.data());
    }

    void parse_args(int argc, char * argv[]) {
      if (mProgramName == "" && argc > 0)
        mProgramName = argv[0];
      for (int i = 1; i < argc; i++) {
        auto tCurrentArgument = argv[i];
        std::map<std::string, std::shared_ptr<Argument>>::iterator tIterator = mArgumentMap.find(argv[i]);
        if (tIterator != mArgumentMap.end()) {
          // Start parsing optional argument
          auto tArgument = tIterator->second;
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
            for (size_t j = 1; j < tCompoundArgument.size(); j++) {
              std::string tArgument(1, tCompoundArgument[j]);
              size_t tNumArgs = 0;
              std::map<std::string, std::shared_ptr<Argument>>::iterator tIterator = mArgumentMap.find("-" + tArgument);
              if (tIterator != mArgumentMap.end()) {
                auto tArgumentObject = tIterator->second;
                tNumArgs = tArgumentObject->mNumArgs;
              }
              std::vector<std::string> tArgumentsForRecursiveParsing = { "", "-" + tArgument };
              while (tNumArgs > 0) {
                i += 1;
                tArgumentsForRecursiveParsing.push_back(argv[i]);
                tNumArgs -= 1;
              }
              parse_args(tArgumentsForRecursiveParsing);
            }
          }
          else {
            // This is a positional argument. 
            // Parse and save into mPositionalArguments vector
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

    template <typename T>
    typename std::enable_if<is_specialization<T, std::vector>::value, T>::type
    get(const char * aArgumentName) {
      std::map<std::string, std::shared_ptr<Argument>>::iterator tIterator = mArgumentMap.find(aArgumentName);
      if (tIterator != mArgumentMap.end()) {
        return tIterator->second->get_vector<T>();
      }
      return T();
    }

    template <typename T>
    typename std::enable_if<is_specialization<T, std::list>::value, T>::type
      get(const char * aArgumentName) {
      std::map<std::string, std::shared_ptr<Argument>>::iterator tIterator = mArgumentMap.find(aArgumentName);
      if (tIterator != mArgumentMap.end()) {
        return tIterator->second->get_list<T>();
      }
      return T();
    }

    std::map<std::string, std::shared_ptr<Argument>> get_arguments() const {
      return mArgumentMap;
    }

    Argument& operator[](const std::string& aArgumentName) {
      std::map<std::string, std::shared_ptr<Argument>>::iterator tIterator = mArgumentMap.find(aArgumentName);
      if (tIterator != mArgumentMap.end()) {
        return *(tIterator->second);
      }
      else {
        throw std::runtime_error("Argument " + aArgumentName + " not found");
      }
    }

    std::string print_help() {
      std::stringstream stream;
      stream << "Usage: " << mProgramName << " [options]";
      size_t tLongestArgumentLength = get_length_of_longest_argument();

      for (size_t i = 0; i < mPositionalArguments.size(); i++) {
        auto tNames = mPositionalArguments[i]->mNames;
        stream << (i == 0 ? " " : "") << tNames[0] << " ";
      }
      stream << "\n\n";

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

      stream << "\nOptional arguments:\n";
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

    bool is_optional(const std::string& aName) {
      return (starts_with(aName, "--") || starts_with(aName, "-"));
    }

    bool is_valid_argument(const std::string& aName) {
      std::map<std::string, std::shared_ptr<Argument>>::iterator tIterator = mArgumentMap.find(aName);
      return (tIterator != mArgumentMap.end());
    }

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
    std::vector<std::shared_ptr<Argument>> mPositionalArguments;
    std::vector<std::shared_ptr<Argument>> mOptionalArguments;
    size_t mNextPositionalArgument;
    std::map<std::string, std::shared_ptr<Argument>> mArgumentMap;
};

}