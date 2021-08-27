/*
  __ _ _ __ __ _ _ __   __ _ _ __ ___  ___
 / _` | '__/ _` | '_ \ / _` | '__/ __|/ _ \ Argument Parser for Modern C++
| (_| | | | (_| | |_) | (_| | |  \__ \  __/ http://github.com/p-ranav/argparse
 \__,_|_|  \__, | .__/ \__,_|_|  |___/\___|
           |___/|_|

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2019-2021 Pranav Srinivas Kumar <pranav.srinivas.kumar@gmail.com>
and other contributors.

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
#include <cerrno>
#include <charconv>
#include <cstdlib>
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

template <typename T, typename = void>
struct is_container : std::false_type {};

template <> struct is_container<std::string> : std::false_type {};

template <typename T>
struct is_container<T, std::void_t<typename T::value_type,
                                   decltype(std::declval<T>().begin()),
                                   decltype(std::declval<T>().end()),
                                   decltype(std::declval<T>().size())>>
  : std::true_type {};

template <typename T>
static constexpr bool is_container_v = is_container<T>::value;

template <typename T, typename = void>
struct is_streamable : std::false_type {};

template <typename T>
struct is_streamable<
    T, std::void_t<decltype(std::declval<std::ostream&>() << std::declval<T>())>>
  : std::true_type {};

template <typename T>
static constexpr bool is_streamable_v = is_streamable<T>::value;

template <typename T>
static constexpr bool is_representable_v =
    is_streamable_v<T> || is_container_v<T>;

constexpr std::size_t repr_max_container_size = 5;

template <typename T> std::string repr(T const &val) {
  if constexpr (std::is_same_v<T, bool>) {
    return val ? "true" : "false";
  } else if constexpr (std::is_convertible_v<T, std::string_view>) {
    return '"' + std::string{std::string_view{val}} + '"';
  } else if constexpr (is_container_v<T>) {
    std::stringstream out;
    out << "{";
    const auto size = val.size();
    if (size > 1) {
      out << repr(*val.begin());
      std::for_each(
          std::next(val.begin()),
          std::next(val.begin(), std::min<std::size_t>(size, repr_max_container_size) - 1),
          [&out](const auto &v) { out << " " << repr(v); });
      if (size <= repr_max_container_size)
        out << " ";
      else
        out << "...";
    }
    if (size > 0)
      out << repr(*std::prev(val.end()));
    out << "}";
    return out.str();
  } else if constexpr (is_streamable_v<T>) {
    std::stringstream out;
    out << val;
    return out.str();
  } else {
    return "<not representable>";
  }
}

namespace {

template <typename T> constexpr bool standard_signed_integer = false;
template <> constexpr bool standard_signed_integer<signed char> = true;
template <> constexpr bool standard_signed_integer<short int> = true;
template <> constexpr bool standard_signed_integer<int> = true;
template <> constexpr bool standard_signed_integer<long int> = true;
template <> constexpr bool standard_signed_integer<long long int> = true;

template <typename T> constexpr bool standard_unsigned_integer = false;
template <> constexpr bool standard_unsigned_integer<unsigned char> = true;
template <> constexpr bool standard_unsigned_integer<unsigned short int> = true;
template <> constexpr bool standard_unsigned_integer<unsigned int> = true;
template <> constexpr bool standard_unsigned_integer<unsigned long int> = true;
template <>
constexpr bool standard_unsigned_integer<unsigned long long int> = true;

} // namespace

template <typename T>
constexpr bool standard_integer =
    standard_signed_integer<T> || standard_unsigned_integer<T>;

template <class F, class Tuple, class Extra, std::size_t... I>
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

constexpr auto pointer_range(std::string_view s) noexcept {
  return std::tuple(s.data(), s.data() + s.size());
}

template <class CharT, class Traits>
constexpr bool starts_with(std::basic_string_view<CharT, Traits> prefix,
                           std::basic_string_view<CharT, Traits> s) noexcept {
  return s.substr(0, prefix.size()) == prefix;
}

enum class chars_format {
  scientific = 0x1,
  fixed = 0x2,
  hex = 0x4,
  general = fixed | scientific
};

struct consume_hex_prefix_result {
  bool is_hexadecimal;
  std::string_view rest;
};

using namespace std::literals;

constexpr auto consume_hex_prefix(std::string_view s)
    -> consume_hex_prefix_result {
  if (starts_with("0x"sv, s) || starts_with("0X"sv, s)) {
    s.remove_prefix(2);
    return {true, s};
  } else {
    return {false, s};
  }
}

template <class T, auto Param>
inline auto do_from_chars(std::string_view s) -> T {
  T x;
  auto [first, last] = pointer_range(s);
  auto [ptr, ec] = std::from_chars(first, last, x, Param);
  if (ec == std::errc()) {
    if (ptr == last)
      return x;
    else
      throw std::invalid_argument{"pattern does not match to the end"};
  } else if (ec == std::errc::invalid_argument) {
    throw std::invalid_argument{"pattern not found"};
  } else if (ec == std::errc::result_out_of_range) {
    throw std::range_error{"not representable"};
  } else {
    return x; // unreachable
  }
}

template <class T, auto Param = 0> struct parse_number {
  auto operator()(std::string_view s) -> T {
    return do_from_chars<T, Param>(s);
  }
};

template <class T> struct parse_number<T, 16> {
  auto operator()(std::string_view s) -> T {
    if (auto [ok, rest] = consume_hex_prefix(s); ok)
      return do_from_chars<T, 16>(rest);
    else
      throw std::invalid_argument{"pattern not found"};
  }
};

template <class T> struct parse_number<T> {
  auto operator()(std::string_view s) -> T {
    if (auto [ok, rest] = consume_hex_prefix(s); ok)
      return do_from_chars<T, 16>(rest);
    else if (starts_with("0"sv, s))
      return do_from_chars<T, 8>(rest);
    else
      return do_from_chars<T, 10>(rest);
  }
};

namespace {

template <class T> constexpr auto generic_strtod = nullptr;
template <> constexpr auto generic_strtod<float> = strtof;
template <> constexpr auto generic_strtod<double> = strtod;
template <> constexpr auto generic_strtod<long double> = strtold;

} // namespace

template <class T> inline auto do_strtod(std::string const &s) -> T {
  if (isspace(static_cast<unsigned char>(s[0])) || s[0] == '+')
    throw std::invalid_argument{"pattern not found"};

  auto [first, last] = pointer_range(s);
  char *ptr;

  errno = 0;
  if (auto x = generic_strtod<T>(first, &ptr); errno == 0) {
    if (ptr == last)
      return x;
    else
      throw std::invalid_argument{"pattern does not match to the end"};
  } else if (errno == ERANGE) {
    throw std::range_error{"not representable"};
  } else {
    return x; // unreachable
  }
}

template <class T> struct parse_number<T, chars_format::general> {
  auto operator()(std::string const &s) -> T {
    if (auto r = consume_hex_prefix(s); r.is_hexadecimal)
      throw std::invalid_argument{
          "chars_format::general does not parse hexfloat"};

    return do_strtod<T>(s);
  }
};

template <class T> struct parse_number<T, chars_format::hex> {
  auto operator()(std::string const &s) -> T {
    if (auto r = consume_hex_prefix(s); !r.is_hexadecimal)
      throw std::invalid_argument{"chars_format::hex parses hexfloat"};

    return do_strtod<T>(s);
  }
};

template <class T> struct parse_number<T, chars_format::scientific> {
  auto operator()(std::string const &s) -> T {
    if (auto r = consume_hex_prefix(s); r.is_hexadecimal)
      throw std::invalid_argument{
          "chars_format::scientific does not parse hexfloat"};
    if (s.find_first_of("eE") == s.npos)
      throw std::invalid_argument{
          "chars_format::scientific requires exponent part"};

    return do_strtod<T>(s);
  }
};

template <class T> struct parse_number<T, chars_format::fixed> {
  auto operator()(std::string const &s) -> T {
    if (auto r = consume_hex_prefix(s); r.is_hexadecimal)
      throw std::invalid_argument{
          "chars_format::fixed does not parse hexfloat"};
    if (s.find_first_of("eE") != s.npos)
      throw std::invalid_argument{
          "chars_format::fixed does not parse exponent part"};

    return do_strtod<T>(s);
  }
};

} // namespace details

class ArgumentParser;

class Argument {
  friend class ArgumentParser;
  friend auto operator<<(std::ostream &, ArgumentParser const &)
      -> std::ostream &;

  template <std::size_t N, std::size_t... I>
  explicit Argument(std::string_view(&&a)[N], std::index_sequence<I...>)
      : mIsOptional((is_optional(a[I]) || ...)), mIsRequired(false),
        mIsRepeatable(false), mIsUsed(false) {
    ((void)mNames.emplace_back(a[I]), ...);
    std::sort(
        mNames.begin(), mNames.end(), [](const auto &lhs, const auto &rhs) {
          return lhs.size() == rhs.size() ? lhs < rhs : lhs.size() < rhs.size();
        });
  }

public:
  template <std::size_t N>
  explicit Argument(std::string_view(&&a)[N])
      : Argument(std::move(a), std::make_index_sequence<N>{}) {}

  Argument &help(std::string aHelp) {
    mHelp = std::move(aHelp);
    return *this;
  }

  template <typename T> Argument &default_value(T &&aDefaultValue) {
    mDefaultValueRepr = details::repr(aDefaultValue);
    mDefaultValue = std::forward<T>(aDefaultValue);
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

  auto &append() {
    mIsRepeatable = true;
    return *this;
  }

  template <char Shape, typename T>
  auto scan() -> std::enable_if_t<std::is_arithmetic_v<T>, Argument &> {
    static_assert(!(std::is_const_v<T> || std::is_volatile_v<T>),
                  "T should not be cv-qualified");
    auto is_one_of = [](char c, auto... x) constexpr {
      return ((c == x) || ...);
    };

    if constexpr (is_one_of(Shape, 'd') && details::standard_integer<T>)
      action(details::parse_number<T, 10>());
    else if constexpr (is_one_of(Shape, 'i') && details::standard_integer<T>)
      action(details::parse_number<T>());
    else if constexpr (is_one_of(Shape, 'u') &&
                       details::standard_unsigned_integer<T>)
      action(details::parse_number<T, 10>());
    else if constexpr (is_one_of(Shape, 'o') &&
                       details::standard_unsigned_integer<T>)
      action(details::parse_number<T, 8>());
    else if constexpr (is_one_of(Shape, 'x', 'X') &&
                       details::standard_unsigned_integer<T>)
      action(details::parse_number<T, 16>());
    else if constexpr (is_one_of(Shape, 'a', 'A') &&
                       std::is_floating_point_v<T>)
      action(details::parse_number<T, details::chars_format::hex>());
    else if constexpr (is_one_of(Shape, 'e', 'E') &&
                       std::is_floating_point_v<T>)
      action(details::parse_number<T, details::chars_format::scientific>());
    else if constexpr (is_one_of(Shape, 'f', 'F') &&
                       std::is_floating_point_v<T>)
      action(details::parse_number<T, details::chars_format::fixed>());
    else if constexpr (is_one_of(Shape, 'g', 'G') &&
                       std::is_floating_point_v<T>)
      action(details::parse_number<T, details::chars_format::general>());
    else
      static_assert(alignof(T) == 0, "No scan specification for T");

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
  Iterator consume(Iterator start, Iterator end,
                   std::string_view usedName = {}) {
    if (!mIsRepeatable && mIsUsed) {
      throw std::runtime_error("Duplicate argument");
    }
    mIsUsed = true;
    mUsedName = usedName;
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
        if (mIsUsed && mValues.size() != *expected && !mIsRepeatable &&
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
          if (!mUsedName.empty())
            stream << mUsedName << ": ";
          stream << *expected << " argument(s) expected. " << mValues.size()
                 << " provided.";
          throw std::runtime_error(stream.str());
        }
      }
    }
  }

  auto maybe_nargs() const -> std::optional<std::size_t> {
    if (mNumArgs < 0)
      return std::nullopt;
    else
      return static_cast<std::size_t>(mNumArgs);
  }

  std::size_t get_arguments_length() const {
    return std::accumulate(std::begin(mNames), std::end(mNames), std::size_t(0),
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
    if (argument.mDefaultValue.has_value()) {
      if (!argument.mHelp.empty())
        stream << " ";
      stream << "[default: " << argument.mDefaultValueRepr << "]";
    } else if (argument.mIsRequired) {
      if (!argument.mHelp.empty())
        stream << " ";
      stream << "[required]";
    }
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
  static constexpr int eof = std::char_traits<char>::eof();

  static auto lookahead(std::string_view s) -> int {
    if (s.empty())
      return eof;
    else
      return static_cast<int>(static_cast<unsigned char>(s[0]));
  }

  /*
   * decimal-literal:
   *    '0'
   *    nonzero-digit digit-sequence_opt
   *    integer-part fractional-part
   *    fractional-part
   *    integer-part '.' exponent-part_opt
   *    integer-part exponent-part
   *
   * integer-part:
   *    digit-sequence
   *
   * fractional-part:
   *    '.' post-decimal-point
   *
   * post-decimal-point:
   *    digit-sequence exponent-part_opt
   *
   * exponent-part:
   *    'e' post-e
   *    'E' post-e
   *
   * post-e:
   *    sign_opt digit-sequence
   *
   * sign: one of
   *    '+' '-'
   */
  static bool is_decimal_literal(std::string_view s) {
    auto is_digit = [](auto c) constexpr {
      switch (c) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        return true;
      default:
        return false;
      }
    };

    // precondition: we have consumed or will consume at least one digit
    auto consume_digits = [=](std::string_view s) {
      auto it = std::find_if_not(std::begin(s), std::end(s), is_digit);
      return s.substr(it - std::begin(s));
    };

    switch (lookahead(s)) {
    case '0': {
      s.remove_prefix(1);
      if (s.empty())
        return true;
      else
        goto integer_part;
    }
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9': {
      s = consume_digits(s);
      if (s.empty())
        return true;
      else
        goto integer_part_consumed;
    }
    case '.': {
      s.remove_prefix(1);
      goto post_decimal_point;
    }
    default:
      return false;
    }

  integer_part:
    s = consume_digits(s);
  integer_part_consumed:
    switch (lookahead(s)) {
    case '.': {
      s.remove_prefix(1);
      if (is_digit(lookahead(s)))
        goto post_decimal_point;
      else
        goto exponent_part_opt;
    }
    case 'e':
    case 'E': {
      s.remove_prefix(1);
      goto post_e;
    }
    default:
      return false;
    }

  post_decimal_point:
    if (is_digit(lookahead(s))) {
      s = consume_digits(s);
      goto exponent_part_opt;
    } else {
      return false;
    }

  exponent_part_opt:
    switch (lookahead(s)) {
    case eof:
      return true;
    case 'e':
    case 'E': {
      s.remove_prefix(1);
      goto post_e;
    }
    default:
      return false;
    }

  post_e:
    switch (lookahead(s)) {
    case '-':
    case '+':
      s.remove_prefix(1);
    }
    if (is_digit(lookahead(s))) {
      s = consume_digits(s);
      return s.empty();
    } else {
      return false;
    }
  }

  static bool is_optional(std::string_view aName) {
    return !is_positional(aName);
  }

  /*
   * positional:
   *    _empty_
   *    '-'
   *    '-' decimal-literal
   *    !'-' anything
   */
  static bool is_positional(std::string_view aName) {
    switch (lookahead(aName)) {
    case eof:
      return true;
    case '-': {
      aName.remove_prefix(1);
      if (aName.empty())
        return true;
      else
        return is_decimal_literal(aName);
    }
    default:
      return true;
    }
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

  /*
   * Get argument value given a type.
   * @pre The object has no default value.
   * @returns The stored value if any, std::nullopt otherwise.
   */
  template <typename T> auto present() const -> std::optional<T> {
    if (mDefaultValue.has_value())
      throw std::logic_error("Argument with default value always presents");

    if (mValues.empty())
      return std::nullopt;
    else if constexpr (details::is_container_v<T>)
      return any_cast_container<T>(mValues);
    else
      return std::any_cast<T>(mValues.front());
  }

  template <typename T>
  static auto any_cast_container(const std::vector<std::any> &aOperand) -> T {
    using ValueType = typename T::value_type;

    T tResult;
    std::transform(
        std::begin(aOperand), std::end(aOperand), std::back_inserter(tResult),
        [](const auto &value) { return std::any_cast<ValueType>(value); });
    return tResult;
  }

  std::vector<std::string> mNames;
  std::string_view mUsedName;
  std::string mHelp;
  std::any mDefaultValue;
  std::string mDefaultValueRepr;
  std::any mImplicitValue;
  using valued_action = std::function<std::any(const std::string &)>;
  using void_action = std::function<void(const std::string &)>;
  std::variant<valued_action, void_action> mAction{
      std::in_place_type<valued_action>,
      [](const std::string &aValue) { return aValue; }};
  std::vector<std::any> mValues;
  int mNumArgs = 1;
  bool mIsOptional : true;
  bool mIsRequired : true;
  bool mIsRepeatable : true;
  bool mIsUsed : true; // True if the optional argument is used by user
};

class ArgumentParser {
public:
  explicit ArgumentParser(std::string aProgramName = {},
                          std::string aVersion = "1.0")
      : mProgramName(std::move(aProgramName)), mVersion(std::move(aVersion)) {
    add_argument("-h", "--help").help("shows help message and exits").nargs(0);
#ifndef ARGPARSE_LONG_VERSION_ARG_ONLY
    add_argument("-v", "--version")
#else
	add_argument("--version")
#endif
        .help("prints version information and exits")
        .nargs(0);
  }

  ArgumentParser(ArgumentParser &&) noexcept = default;
  ArgumentParser &operator=(ArgumentParser &&) = default;

  ArgumentParser(const ArgumentParser &other)
      : mProgramName(other.mProgramName),
        mPositionalArguments(other.mPositionalArguments),
        mOptionalArguments(other.mOptionalArguments) {
    for (auto it = std::begin(mPositionalArguments); it != std::end(mPositionalArguments);
         ++it)
      index_argument(it);
    for (auto it = std::begin(mOptionalArguments); it != std::end(mOptionalArguments);
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
    using array_of_sv = std::string_view[sizeof...(Targs)];
    auto tArgument = mOptionalArguments.emplace(cend(mOptionalArguments),
                                                array_of_sv{Fargs...});

    if (!tArgument->mIsOptional)
      mPositionalArguments.splice(cend(mPositionalArguments),
                                  mOptionalArguments, tArgument);

    index_argument(tArgument);
    return *tArgument;
  }

  // Parameter packed add_parents method
  // Accepts a variadic number of ArgumentParser objects
  template <typename... Targs>
  ArgumentParser &add_parents(const Targs &... Fargs) {
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
    return *this;
  }

  ArgumentParser &add_description(std::string aDescription) {
    mDescription = std::move(aDescription);
    return *this;
  }

  ArgumentParser &add_epilog(std::string aEpilog) {
    mEpilog = std::move(aEpilog);
    return *this;
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

  /* Getter for options with default values.
   * @throws std::logic_error if there is no such option
   * @throws std::logic_error if the option has no value
   * @throws std::bad_any_cast if the option is not of type T
   */
  template <typename T = std::string>
  T get(std::string_view aArgumentName) const {
    return (*this)[aArgumentName].get<T>();
  }

  /* Getter for options without default values.
   * @pre The option has no default value.
   * @throws std::logic_error if there is no such option
   * @throws std::bad_any_cast if the option is not of type T
   */
  template <typename T = std::string>
  auto present(std::string_view aArgumentName) const -> std::optional<T> {
    return (*this)[aArgumentName].present<T>();
  }

  /* Getter that returns true for user-supplied options. Returns false if not
   * user-supplied, even with a default value.
   */
  auto is_used(std::string_view aArgumentName) const {
    return (*this)[aArgumentName].mIsUsed;
  }

  /* Indexing operator. Return a reference to an Argument object
   * Used in conjuction with Argument.operator== e.g., parser["foo"] == true
   * @throws std::logic_error in case of an invalid argument name
   */
  Argument &operator[](std::string_view aArgumentName) const {
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
      std::size_t tLongestArgumentLength = parser.get_length_of_longest_argument();

      for (const auto &argument : parser.mPositionalArguments) {
        stream << argument.mNames.front() << " ";
      }
      stream << "\n\n";

      if (!parser.mDescription.empty())
        stream << parser.mDescription << "\n\n";

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

      if (!parser.mEpilog.empty())
        stream << parser.mEpilog << "\n\n";
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
      if (Argument::is_positional(tCurrentArgument)) {
        if (positionalArgumentIt == std::end(mPositionalArguments)) {
          throw std::runtime_error(
              "Maximum number of positional arguments exceeded");
        }
        auto tArgument = positionalArgumentIt++;
        it = tArgument->consume(it, end);
        continue;
      }

      auto tIterator = mArgumentMap.find(tCurrentArgument);
      if (tIterator != mArgumentMap.end()) {
        auto tArgument = tIterator->second;

        // the first optional argument is --help
        if (tArgument == mOptionalArguments.begin()) {
          std::cout << *this;
          std::exit(0);
        }
        // the second optional argument is --version
        else if (tArgument == std::next(mOptionalArguments.begin(), 1)) {
          std::cout << mVersion << "\n";
          std::exit(0);
        }

        it = tArgument->consume(std::next(it), end, tIterator->first);
      } else if (const auto &tCompoundArgument = tCurrentArgument;
                 tCompoundArgument.size() > 1 && tCompoundArgument[0] == '-' &&
                 tCompoundArgument[1] != '-') {
        ++it;
        for (std::size_t j = 1; j < tCompoundArgument.size(); j++) {
          auto tHypotheticalArgument = std::string{'-', tCompoundArgument[j]};
          auto tIterator2 = mArgumentMap.find(tHypotheticalArgument);
          if (tIterator2 != mArgumentMap.end()) {
            auto tArgument = tIterator2->second;
            it = tArgument->consume(it, end, tIterator2->first);
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
  std::size_t get_length_of_longest_argument() const {
    if (mArgumentMap.empty())
      return 0;
    std::vector<std::size_t> argumentLengths(mArgumentMap.size());
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
      mArgumentMap.insert_or_assign(mName, argIt);
  }

  std::string mProgramName;
  std::string mVersion;
  std::string mDescription;
  std::string mEpilog;
  std::list<Argument> mPositionalArguments;
  std::list<Argument> mOptionalArguments;
  std::map<std::string_view, list_iterator, std::less<>> mArgumentMap;
};

} // namespace argparse
