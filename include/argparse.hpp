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
#include <numeric>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <variant>
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
                       void>> : std::true_type {};

template <typename T>
static constexpr bool is_container_v = is_container<T>::value;

template <typename T>
struct is_string_like
    : std::conjunction<std::is_constructible<std::string, T>,
                       std::is_convertible<T, std::string_view>> {};

template <class F, class Tuple, class Extra, size_t... I>
constexpr decltype(auto) apply_plus_one_impl(F &&f, Tuple &&t, Extra &&x,
                                             std::index_sequence<I...>) {
  return std::invoke(std::forward<F>(f), std::get<I>(std::forward<Tuple>(t))...,
                     std::forward<Extra>(x));
}

template <class F, class Tuple, class Extra>
constexpr decltype(auto) apply_plus_one(F &&f, Tuple &&t, Extra &&x) {
  return details::apply_plus_one_impl(
      std::forward<F>(f), std::forward<Tuple>(t), std::forward<Extra>(x),
      std::make_index_sequence<
          std::tuple_size_v<std::remove_reference_t<Tuple>>>{});
}

} // namespace details

class ArgumentParser;

class Argument {
  friend class ArgumentParser;
  friend auto operator<<(std::ostream &, ArgumentParser const &)
      -> std::ostream &;

  template <size_t N, size_t... I>
  explicit Argument(std::string(&&a)[N], std::index_sequence<I...>)
      : mIsOptional((is_optional(a[I]) || ...)), mIsRequired(false),
        mIsUsed(false) {
    ((void)mNames.push_back(std::move(a[I])), ...);
    std::sort(
        mNames.begin(), mNames.end(), [](const auto &lhs, const auto &rhs) {
          return lhs.size() == rhs.size() ? lhs < rhs : lhs.size() < rhs.size();
        });
  }

public:
  Argument() = default;

  template <typename... Args,
            std::enable_if_t<
                std::conjunction_v<details::is_string_like<Args>...>, int> = 0>
  explicit Argument(Args &&... args)
      : Argument({std::string(std::forward<Args>(args))...},
                 std::make_index_sequence<sizeof...(Args)>{}) {}

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

  template <class F, class... Args>
  auto action(F &&aAction, Args &&... aBound)
      -> std::enable_if_t<std::is_invocable_v<F, Args..., std::string const>,
                          Argument &> {
    using action_type = std::conditional_t<
        std::is_void_v<std::invoke_result_t<F, Args..., std::string const>>,
        void_action, valued_action>;
    if constexpr (sizeof...(Args) == 0)
      mAction.emplace<action_type>(std::forward<F>(aAction));
    else
      mAction.emplace<action_type>(
          [f = std::forward<F>(aAction),
           tup = std::make_tuple(std::forward<Args>(aBound)...)](
              std::string const &opt) mutable {
            return details::apply_plus_one(f, tup, opt);
          });
    return *this;
  }

  Argument &nargs(int aNumArgs) {
    if (aNumArgs < 0)
      throw std::logic_error("Number of arguments must be non-negative");
    mNumArgs = aNumArgs;
    return *this;
  }

  Argument &remaining() {
    mNumArgs = -1;
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
    } else if (mNumArgs <= std::distance(start, end)) {
      if (auto expected = maybe_nargs()) {
        end = std::next(start, *expected);
        if (std::any_of(start, end, Argument::is_optional)) {
          throw std::runtime_error("optional argument in parameter sequence");
        }
      }

      struct action_apply {
        void operator()(valued_action &f) {
          std::transform(start, end, std::back_inserter(self.mValues), f);
        }

        void operator()(void_action &f) {
          std::for_each(start, end, f);
          if (!self.mDefaultValue.has_value()) {
            if (auto expected = self.maybe_nargs())
              self.mValues.resize(*expected);
          }
        }

        Iterator start, end;
        Argument &self;
      };
      std::visit(action_apply{start, end, *this}, mAction);
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
    if (auto expected = maybe_nargs()) {
      if (mIsOptional) {
        if (mIsUsed && mValues.size() != *expected &&
            !mDefaultValue.has_value()) {
          std::stringstream stream;
          stream << mUsedName << ": expected " << *expected << " argument(s). "
                 << mValues.size() << " provided.";
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
        if (mValues.size() != expected && !mDefaultValue.has_value()) {
          std::stringstream stream;
          stream << mUsedName << ": expected " << *expected << " argument(s). "
                 << mValues.size() << " provided.";
          throw std::runtime_error(stream.str());
        }
      }
    }
  }

  auto maybe_nargs() const -> std::optional<size_t> {
    if (mNumArgs < 0)
      return std::nullopt;
    else
      return static_cast<size_t>(mNumArgs);
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
   * Compare to an argument value of known type
   * @throws std::logic_error in case of incompatible types
   */
  template <typename T> bool operator==(const T &aRhs) const {
    if constexpr (!details::is_container_v<T>) {
      return get<T>() == aRhs;
    } else {
      using ValueType = typename T::value_type;
      auto tLhs = get<T>();
      return std::equal(std::begin(tLhs), std::end(tLhs), std::begin(aRhs),
                        std::end(aRhs), [](const auto &lhs, const auto &rhs) {
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
    return (aName.size() > 1 && aName[0] == '-' && !is_integer(aName) &&
            !is_float(aName));
  }

  static bool is_positional(const std::string &aName) {
    return !is_optional(aName);
  }

  /*
   * Get argument value given a type
   * @throws std::logic_error in case of incompatible types
   */
  template <typename T> T get() const {
    if (!mValues.empty()) {
      if constexpr (details::is_container_v<T>)
        return any_cast_container<T>(mValues);
      else
        return std::any_cast<T>(mValues.front());
    }
    if (mDefaultValue.has_value()) {
      return std::any_cast<T>(mDefaultValue);
    }
    throw std::logic_error("No value provided");
  }

  template <typename T>
  static auto any_cast_container(const std::vector<std::any> &aOperand) -> T {
    using ValueType = typename T::value_type;

    T tResult;
    std::transform(
        begin(aOperand), end(aOperand), std::back_inserter(tResult),
        [](const auto &value) { return std::any_cast<ValueType>(value); });
    return tResult;
  }

  std::vector<std::string> mNames;
  std::string mUsedName;
  std::string mHelp;
  std::any mDefaultValue;
  std::any mImplicitValue;
  using valued_action = std::function<std::any(const std::string &)>;
  using void_action = std::function<void(const std::string &)>;
  std::variant<valued_action, void_action> mAction{
      std::in_place_type<valued_action>,
      [](const std::string &aValue) { return aValue; }};
  std::vector<std::any> mValues;
  int mNumArgs = 1;
  bool mIsOptional : 1;
  bool mIsRequired : 1;
  bool mIsUsed : 1; // True if the optional argument is used by user

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

  ArgumentParser(ArgumentParser &&) noexcept = default;
  ArgumentParser &operator=(ArgumentParser &&) = default;

  ArgumentParser(const ArgumentParser &other)
      : mProgramName(other.mProgramName),
        mPositionalArguments(other.mPositionalArguments),
        mOptionalArguments(other.mOptionalArguments) {
    for (auto it = begin(mPositionalArguments); it != end(mPositionalArguments);
         ++it)
      index_argument(it);
    for (auto it = begin(mOptionalArguments); it != end(mOptionalArguments);
         ++it)
      index_argument(it);
  }

  ArgumentParser &operator=(const ArgumentParser &other) {
    auto tmp = other;
    std::swap(*this, tmp);
    return *this;
  }

  // Parameter packing
  // Call add_argument with variadic number of string arguments
  template <typename... Targs> Argument &add_argument(Targs... Fargs) {
    auto tArgument = mOptionalArguments.emplace(cend(mOptionalArguments),
                                                std::move(Fargs)...);

    if (!tArgument->mIsOptional)
      mPositionalArguments.splice(cend(mPositionalArguments),
                                  mOptionalArguments, tArgument);

    index_argument(tArgument);
    return *tArgument;
  }

  // Parameter packed add_parents method
  // Accepts a variadic number of ArgumentParser objects
  template <typename... Targs> void add_parents(const Targs &... Fargs) {
    for (const ArgumentParser &tParentParser : {std::ref(Fargs)...}) {
      for (auto &tArgument : tParentParser.mPositionalArguments) {
        auto it =
            mPositionalArguments.insert(cend(mPositionalArguments), tArgument);
        index_argument(it);
      }
      for (auto &tArgument : tParentParser.mOptionalArguments) {
        auto it =
            mOptionalArguments.insert(cend(mOptionalArguments), tArgument);
        index_argument(it);
      }
    }
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
  template <typename T = std::string> T get(std::string_view aArgumentName) {
    return (*this)[aArgumentName].get<T>();
  }

  /* Indexing operator. Return a reference to an Argument object
   * Used in conjuction with Argument.operator== e.g., parser["foo"] == true
   * @throws std::logic_error in case of an invalid argument name
   */
  Argument &operator[](std::string_view aArgumentName) {
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
        stream << argument.mNames.front() << " ";
      }
      stream << "\n\n";

      if (!parser.mPositionalArguments.empty())
        stream << "Positional arguments:\n";

      for (const auto &mPositionalArgument : parser.mPositionalArguments) {
        stream.width(tLongestArgumentLength);
        stream << mPositionalArgument;
      }

      if (!parser.mOptionalArguments.empty())
        stream << (parser.mPositionalArguments.empty() ? "" : "\n")
               << "Optional arguments:\n";

      for (const auto &mOptionalArgument : parser.mOptionalArguments) {
        stream.width(tLongestArgumentLength);
        stream << mOptionalArgument;
      }
    }

    return stream;
  }

  // Format help message
  auto help() const -> std::stringstream {
    std::stringstream out;
    out << *this;
    return out;
  }

  // Printing the one and only help message
  // I've stuck with a simple message format, nothing fancy.
  [[deprecated("Use cout << program; instead.  See also help().")]] std::string
  print_help() {
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
        auto tArgument = positionalArgumentIt++;
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
          auto tHypotheticalArgument = std::string{'-', tCompoundArgument[j]};
          auto tIterator2 = mArgumentMap.find(tHypotheticalArgument);
          if (tIterator2 != mArgumentMap.end()) {
            auto tArgument = tIterator2->second;
            it = tArgument->consume(it, end, tHypotheticalArgument);
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

  using list_iterator = std::list<Argument>::iterator;

  void index_argument(list_iterator argIt) {
    for (auto &mName : std::as_const(argIt->mNames))
      mArgumentMap.emplace(mName, argIt);
  }

  std::string mProgramName;
  std::list<Argument> mPositionalArguments;
  std::list<Argument> mOptionalArguments;
  std::map<std::string_view, list_iterator, std::less<>> mArgumentMap;
};

} // namespace argparse
