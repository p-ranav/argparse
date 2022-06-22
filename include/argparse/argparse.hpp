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
#include <array>
#include <cerrno>
#include <charconv>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <iterator>
#include <limits>
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
#include <utility>
#include <variant>
#include <vector>

namespace argparse {

namespace details { // namespace for helper methods

template <typename T, typename = void>
struct HasContainerTraits : std::false_type {};

template <> struct HasContainerTraits<std::string> : std::false_type {};

template <typename T>
struct HasContainerTraits<
    T, std::void_t<typename T::value_type, decltype(std::declval<T>().begin()),
                   decltype(std::declval<T>().end()),
                   decltype(std::declval<T>().size())>> : std::true_type {};

template <typename T>
static constexpr bool IsContainer = HasContainerTraits<T>::value;

template <typename T, typename = void>
struct HasStreamableTraits : std::false_type {};

template <typename T>
struct HasStreamableTraits<
    T,
    std::void_t<decltype(std::declval<std::ostream &>() << std::declval<T>())>>
    : std::true_type {};

template <typename T>
static constexpr bool IsStreamable = HasStreamableTraits<T>::value;

constexpr std::size_t repr_max_container_size = 5;

template <typename T> std::string repr(T const &val) {
  if constexpr (std::is_same_v<T, bool>) {
    return val ? "true" : "false";
  } else if constexpr (std::is_convertible_v<T, std::string_view>) {
    return '"' + std::string{std::string_view{val}} + '"';
  } else if constexpr (IsContainer<T>) {
    std::stringstream out;
    out << "{";
    const auto size = val.size();
    if (size > 1) {
      out << repr(*val.begin());
      std::for_each(
          std::next(val.begin()),
          std::next(val.begin(),
                    std::min<std::size_t>(size, repr_max_container_size) - 1),
          [&out](const auto &v) { out << " " << repr(v); });
      if (size <= repr_max_container_size) {
        out << " ";
      } else {
        out << "...";
      }
    }
    if (size > 0) {
      out << repr(*std::prev(val.end()));
    }
    out << "}";
    return out.str();
  } else if constexpr (IsStreamable<T>) {
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

constexpr int radix_8 = 8;
constexpr int radix_10 = 10;
constexpr int radix_16 = 16;

template <typename T>
constexpr bool standard_integer =
    standard_signed_integer<T> || standard_unsigned_integer<T>;

template <class F, class Tuple, class Extra, std::size_t... I>
constexpr decltype(auto) apply_plus_one_impl(F &&f, Tuple &&t, Extra &&x,
                                             std::index_sequence<I...> /*unused*/) {
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

struct ConsumeHexPrefixResult {
  bool is_hexadecimal;
  std::string_view rest;
};

using namespace std::literals;

constexpr auto consume_hex_prefix(std::string_view s)
    -> ConsumeHexPrefixResult {
  if (starts_with("0x"sv, s) || starts_with("0X"sv, s)) {
    s.remove_prefix(2);
    return {true, s};
  }
  return {false, s};
}

template <class T, auto Param>
inline auto do_from_chars(std::string_view s) -> T {
  T x;
  auto [first, last] = pointer_range(s);
  auto [ptr, ec] = std::from_chars(first, last, x, Param);
  if (ec == std::errc()) {
    if (ptr == last) {
      return x;
    }
    throw std::invalid_argument{"pattern does not match to the end"};
  }
  if (ec == std::errc::invalid_argument) {
    throw std::invalid_argument{"pattern not found"};
  }
  if (ec == std::errc::result_out_of_range) {
    throw std::range_error{"not representable"};
  }
  return x; // unreachable
}

template <class T, auto Param = 0> struct parse_number {
  auto operator()(std::string_view s) -> T {
    return do_from_chars<T, Param>(s);
  }
};

template <class T> struct parse_number<T, radix_16> {
  auto operator()(std::string_view s) -> T {
    if (auto [ok, rest] = consume_hex_prefix(s); ok) {
      return do_from_chars<T, radix_16>(rest);
    }
    throw std::invalid_argument{"pattern not found"};
  }
};

template <class T> struct parse_number<T> {
  auto operator()(std::string_view s) -> T {
    auto [ok, rest] = consume_hex_prefix(s);
    if (ok) {
      return do_from_chars<T, radix_16>(rest);
    }
    if (starts_with("0"sv, s)) {
      return do_from_chars<T, radix_8>(rest);
    }
    return do_from_chars<T, radix_10>(rest);
  }
};

namespace {

template <class T> constexpr auto generic_strtod = nullptr;
template <> constexpr auto generic_strtod<float> = strtof;
template <> constexpr auto generic_strtod<double> = strtod;
template <> constexpr auto generic_strtod<long double> = strtold;

} // namespace

template <class T> inline auto do_strtod(std::string const &s) -> T {
  if (isspace(static_cast<unsigned char>(s[0])) || s[0] == '+') {
    throw std::invalid_argument{"pattern not found"};
  }

  auto [first, last] = pointer_range(s);
  char *ptr;

  errno = 0;
  auto x = generic_strtod<T>(first, &ptr);
  if (errno == 0) {
    if (ptr == last) {
      return x;
    }
    throw std::invalid_argument{"pattern does not match to the end"};
  }
  if (errno == ERANGE) {
    throw std::range_error{"not representable"};
  }
  return x; // unreachable
}

template <class T> struct parse_number<T, chars_format::general> {
  auto operator()(std::string const &s) -> T {
    if (auto r = consume_hex_prefix(s); r.is_hexadecimal) {
      throw std::invalid_argument{
          "chars_format::general does not parse hexfloat"};
    }

    return do_strtod<T>(s);
  }
};

template <class T> struct parse_number<T, chars_format::hex> {
  auto operator()(std::string const &s) -> T {
    if (auto r = consume_hex_prefix(s); !r.is_hexadecimal) {
      throw std::invalid_argument{"chars_format::hex parses hexfloat"};
    }

    return do_strtod<T>(s);
  }
};

template <class T> struct parse_number<T, chars_format::scientific> {
  auto operator()(std::string const &s) -> T {
    if (auto r = consume_hex_prefix(s); r.is_hexadecimal) {
      throw std::invalid_argument{
          "chars_format::scientific does not parse hexfloat"};
    }
    if (s.find_first_of("eE") == std::string::npos) {
      throw std::invalid_argument{
          "chars_format::scientific requires exponent part"};
    }

    return do_strtod<T>(s);
  }
};

template <class T> struct parse_number<T, chars_format::fixed> {
  auto operator()(std::string const &s) -> T {
    if (auto r = consume_hex_prefix(s); r.is_hexadecimal) {
      throw std::invalid_argument{
          "chars_format::fixed does not parse hexfloat"};
    }
    if (s.find_first_of("eE") != std::string::npos) {
      throw std::invalid_argument{
          "chars_format::fixed does not parse exponent part"};
    }

    return do_strtod<T>(s);
  }
};

} // namespace details

enum class nargs_pattern {
  optional,
  any,
  at_least_one
};

enum class default_arguments : unsigned int {
  none = 0,
  help = 1,
  version = 2,
  all = help | version,
};

inline default_arguments operator&(const default_arguments &a,
                                   const default_arguments &b) {
  return static_cast<default_arguments>(
      static_cast<std::underlying_type<default_arguments>::type>(a) &
      static_cast<std::underlying_type<default_arguments>::type>(b));
}

class ArgumentParser;

class Argument {
  friend class ArgumentParser;
  friend auto operator<<(std::ostream &stream, const ArgumentParser &parser)
      -> std::ostream &;

  template <std::size_t N, std::size_t... I>
  explicit Argument(std::array<std::string_view, N> &&a,
                    std::index_sequence<I...> /*unused*/)
      : m_is_optional((is_optional(a[I]) || ...)), m_is_required(false),
        m_is_repeatable(false), m_is_used(false) {
    ((void)m_names.emplace_back(a[I]), ...);
    std::sort(
        m_names.begin(), m_names.end(), [](const auto &lhs, const auto &rhs) {
          return lhs.size() == rhs.size() ? lhs < rhs : lhs.size() < rhs.size();
        });
  }

public:
  template <std::size_t N>
  explicit Argument(std::array<std::string_view, N> &&a)
      : Argument(std::move(a), std::make_index_sequence<N>{}) {}

  Argument &help(std::string help_text) {
    m_help = std::move(help_text);
    return *this;
  }

  template <typename T> Argument &default_value(T &&value) {
    m_default_value_repr = details::repr(value);
    m_default_value = std::forward<T>(value);
    return *this;
  }

  Argument &required() {
    m_is_required = true;
    return *this;
  }

  Argument &implicit_value(std::any value) {
    m_implicit_value = std::move(value);
    m_num_args_range = NArgsRange{0, 0};
    return *this;
  }

  template <class F, class... Args>
  auto action(F &&callable, Args &&...bound_args)
      -> std::enable_if_t<std::is_invocable_v<F, Args..., std::string const>,
                          Argument &> {
    using action_type = std::conditional_t<
        std::is_void_v<std::invoke_result_t<F, Args..., std::string const>>,
        void_action, valued_action>;
    if constexpr (sizeof...(Args) == 0) {
      m_action.emplace<action_type>(std::forward<F>(callable));
    } else {
      m_action.emplace<action_type>(
          [f = std::forward<F>(callable),
           tup = std::make_tuple(std::forward<Args>(bound_args)...)](
              std::string const &opt) mutable {
            return details::apply_plus_one(f, tup, opt);
          });
    }
    return *this;
  }

  auto &append() {
    m_is_repeatable = true;
    return *this;
  }

  template <char Shape, typename T>
  auto scan() -> std::enable_if_t<std::is_arithmetic_v<T>, Argument &> {
    static_assert(!(std::is_const_v<T> || std::is_volatile_v<T>),
                  "T should not be cv-qualified");
    auto is_one_of = [](char c, auto... x) constexpr {
      return ((c == x) || ...);
    };

    if constexpr (is_one_of(Shape, 'd') && details::standard_integer<T>) {
      action(details::parse_number<T, details::radix_10>());
    } else if constexpr (is_one_of(Shape, 'i') && details::standard_integer<T>) {
      action(details::parse_number<T>());
    } else if constexpr (is_one_of(Shape, 'u') &&
                         details::standard_unsigned_integer<T>) {
      action(details::parse_number<T, details::radix_10>());
    } else if constexpr (is_one_of(Shape, 'o') &&
                         details::standard_unsigned_integer<T>) {
      action(details::parse_number<T, details::radix_8>());
    } else if constexpr (is_one_of(Shape, 'x', 'X') &&
                         details::standard_unsigned_integer<T>) {
      action(details::parse_number<T, details::radix_16>());
    } else if constexpr (is_one_of(Shape, 'a', 'A') &&
                         std::is_floating_point_v<T>) {
      action(details::parse_number<T, details::chars_format::hex>());
    } else if constexpr (is_one_of(Shape, 'e', 'E') &&
                         std::is_floating_point_v<T>) {
      action(details::parse_number<T, details::chars_format::scientific>());
    } else if constexpr (is_one_of(Shape, 'f', 'F') &&
                         std::is_floating_point_v<T>) {
      action(details::parse_number<T, details::chars_format::fixed>());
    } else if constexpr (is_one_of(Shape, 'g', 'G') &&
                         std::is_floating_point_v<T>) {
      action(details::parse_number<T, details::chars_format::general>());
    } else {
      static_assert(alignof(T) == 0, "No scan specification for T");
    }

    return *this;
  }

  Argument &nargs(std::size_t num_args) {
    m_num_args_range = NArgsRange{num_args, num_args};
    return *this;
  }

  Argument &nargs(std::size_t num_args_min, std::size_t num_args_max) {
    m_num_args_range = NArgsRange{num_args_min, num_args_max};
    return *this;
  }

  Argument &nargs(nargs_pattern pattern) {
    switch (pattern) {
    case nargs_pattern::optional:
      m_num_args_range = NArgsRange{0, 1};
      break;
    case nargs_pattern::any:
      m_num_args_range = NArgsRange{0, std::numeric_limits<std::size_t>::max()};
      break;
    case nargs_pattern::at_least_one:
      m_num_args_range = NArgsRange{1, std::numeric_limits<std::size_t>::max()};
      break;
    }
    return *this;
  }

  Argument &remaining() {
    m_accepts_optional_like_value = true;
    return nargs(nargs_pattern::any);
  }

  template <typename Iterator>
  Iterator consume(Iterator start, Iterator end,
                   std::string_view used_name = {}) {
    if (!m_is_repeatable && m_is_used) {
      throw std::runtime_error("Duplicate argument");
    }
    m_is_used = true;
    m_used_name = used_name;

    const auto num_args_max = m_num_args_range.get_max();
    const auto num_args_min = m_num_args_range.get_min();
    std::size_t dist = 0;
    if (num_args_max == 0) {
      m_values.emplace_back(m_implicit_value);
      std::visit([](const auto &f) { f({}); }, m_action);
      return start;
    } else if ((dist = static_cast<std::size_t>(std::distance(start, end))) >= num_args_min) {
      if (num_args_max < dist) {
        end = std::next(start, num_args_max);
      }
      if (!m_accepts_optional_like_value) {
        end = std::find_if(start, end, Argument::is_optional);
        dist = static_cast<std::size_t>(std::distance(start, end));
        if (dist < num_args_min) {
          throw std::runtime_error("Too few arguments");
        }
      }

      struct ActionApply {
        void operator()(valued_action &f) {
          std::transform(first, last, std::back_inserter(self.m_values), f);
        }

        void operator()(void_action &f) {
          std::for_each(first, last, f);
          if (!self.m_default_value.has_value()) {
            if (!self.m_accepts_optional_like_value)
              self.m_values.resize(std::distance(first, last));
          }
        }

        Iterator first, last;
        Argument &self;
      };
      std::visit(ActionApply{start, end, *this}, m_action);
      return end;
    }
    if (m_default_value.has_value()) {
      return start;
    }
    throw std::runtime_error("Too few arguments for '" +
                             std::string(m_used_name) + "'.");
  }

  /*
   * @throws std::runtime_error if argument values are not valid
   */
  void validate() const {
    if (m_is_optional) {
      // TODO: check if an implicit value was programmed for this argument
      if (!m_is_used && !m_default_value.has_value() && m_is_required) {
        throw_required_arg_not_used_error();
      }
      if (m_is_used && m_is_required && m_values.empty()) {
        throw_required_arg_no_value_provided_error();
      }
    } else {
      if (!m_num_args_range.contains(m_values.size()) && !m_default_value.has_value()) {
        throw_nargs_range_validation_error();
      }
    }
  }

  std::size_t get_arguments_length() const {
    return std::accumulate(std::begin(m_names), std::end(m_names),
                           std::size_t(0), [](const auto &sum, const auto &s) {
                             return sum + s.size() +
                                    1; // +1 for space between names
                           });
  }

  friend std::ostream &operator<<(std::ostream &stream,
                                  const Argument &argument) {
    std::stringstream name_stream;
    std::copy(std::begin(argument.m_names), std::end(argument.m_names),
              std::ostream_iterator<std::string>(name_stream, " "));
    stream << name_stream.str() << "\t" << argument.m_help;
    if (argument.m_default_value.has_value()) {
      if (!argument.m_help.empty()) {
        stream << " ";
      }
      stream << "[default: " << argument.m_default_value_repr << "]";
    } else if (argument.m_is_required) {
      if (!argument.m_help.empty()) {
        stream << " ";
      }
      stream << "[required]";
    }
    stream << "\n";
    return stream;
  }

  template <typename T> bool operator!=(const T &rhs) const {
    return !(*this == rhs);
  }

  /*
   * Compare to an argument value of known type
   * @throws std::logic_error in case of incompatible types
   */
  template <typename T> bool operator==(const T &rhs) const {
    if constexpr (!details::IsContainer<T>) {
      return get<T>() == rhs;
    } else {
      using ValueType = typename T::value_type;
      auto lhs = get<T>();
      return std::equal(std::begin(lhs), std::end(lhs), std::begin(rhs),
                        std::end(rhs), [](const auto &lhs, const auto &rhs) {
                          return std::any_cast<const ValueType &>(lhs) == rhs;
                        });
    }
  }

private:

  class NArgsRange {
    std::size_t m_min;
    std::size_t m_max;

  public:
    NArgsRange(std::size_t minimum, std::size_t maximum) : m_min(minimum), m_max(maximum) {
      if (minimum > maximum)
        throw std::logic_error("Range of number of arguments is invalid");
    }

    bool contains(std::size_t value) const {
      return value >= m_min && value <= m_max;
    }

    bool is_exact() const {
      return m_min == m_max;
    }

    bool is_right_bounded() const {
      return m_max < std::numeric_limits<std::size_t>::max();
    }

    std::size_t get_min() const {
      return m_min;
    }

    std::size_t get_max() const {
      return m_max;
    }
  };

  void throw_nargs_range_validation_error() const {
    std::stringstream stream;
    if (!m_used_name.empty())
      stream << m_used_name << ": ";
    if (m_num_args_range.is_exact()) {
      stream << m_num_args_range.get_min();
    } else if (m_num_args_range.is_right_bounded()) {
      stream << m_num_args_range.get_min() << " to " << m_num_args_range.get_max();
    } else {
      stream << m_num_args_range.get_min() << " or more";
    }
    stream << " argument(s) expected. "
      << m_values.size() << " provided.";
    throw std::runtime_error(stream.str());
  }

  void throw_required_arg_not_used_error() const {
    std::stringstream stream;
    stream << m_names[0] << ": required.";
    throw std::runtime_error(stream.str());
  }

  void throw_required_arg_no_value_provided_error() const {
    std::stringstream stream;
    stream << m_used_name << ": no value provided.";
    throw std::runtime_error(stream.str());
  }

  static constexpr int eof = std::char_traits<char>::eof();

  static auto lookahead(std::string_view s) -> int {
    if (s.empty()) {
      return eof;
    }
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
      // NOLINTNEXTLINE(readability-qualified-auto)
      auto it = std::find_if_not(std::begin(s), std::end(s), is_digit);
      return s.substr(it - std::begin(s));
    };

    switch (lookahead(s)) {
    case '0': {
      s.remove_prefix(1);
      if (s.empty()) {
        return true;
      }
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
      if (s.empty()) {
        return true;
      }
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
      if (is_digit(lookahead(s))) {
        goto post_decimal_point;
      } else {
        goto exponent_part_opt;
      }
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
    }
    return false;

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
    }
    return false;
  }

  static bool is_optional(std::string_view name) {
    return !is_positional(name);
  }

  /*
   * positional:
   *    _empty_
   *    '-'
   *    '-' decimal-literal
   *    !'-' anything
   */
  static bool is_positional(std::string_view name) {
    switch (lookahead(name)) {
    case eof:
      return true;
    case '-': {
      name.remove_prefix(1);
      if (name.empty()) {
        return true;
      }
      return is_decimal_literal(name);
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
    if (!m_values.empty()) {
      if constexpr (details::IsContainer<T>) {
        return any_cast_container<T>(m_values);
      } else {
        return std::any_cast<T>(m_values.front());
      }
    }
    if (m_default_value.has_value()) {
      return std::any_cast<T>(m_default_value);
    } else {
      if constexpr (details::IsContainer<T>)
        if (!m_accepts_optional_like_value)
          return any_cast_container<T>(m_values);
    }
    throw std::logic_error("No value provided for '" + m_names.back() + "'.");
  }

  /*
   * Get argument value given a type.
   * @pre The object has no default value.
   * @returns The stored value if any, std::nullopt otherwise.
   */
  template <typename T> auto present() const -> std::optional<T> {
    if (m_default_value.has_value()) {
      throw std::logic_error("Argument with default value always presents");
    }
    if (m_values.empty()) {
      return std::nullopt;
    }
    if constexpr (details::IsContainer<T>) {
      return any_cast_container<T>(m_values);
    }
    return std::any_cast<T>(m_values.front());
  }

  template <typename T>
  static auto any_cast_container(const std::vector<std::any> &operand) -> T {
    using ValueType = typename T::value_type;

    T result;
    std::transform(
        std::begin(operand), std::end(operand), std::back_inserter(result),
        [](const auto &value) { return std::any_cast<ValueType>(value); });
    return result;
  }

  std::vector<std::string> m_names;
  std::string_view m_used_name;
  std::string m_help;
  std::any m_default_value;
  std::string m_default_value_repr;
  std::any m_implicit_value;
  using valued_action = std::function<std::any(const std::string &)>;
  using void_action = std::function<void(const std::string &)>;
  std::variant<valued_action, void_action> m_action{
      std::in_place_type<valued_action>,
      [](const std::string &value) { return value; }};
  std::vector<std::any> m_values;
  NArgsRange m_num_args_range {1, 1};
  bool m_accepts_optional_like_value = false;
  bool m_is_optional : true;
  bool m_is_required : true;
  bool m_is_repeatable : true;
  bool m_is_used : true; // True if the optional argument is used by user
};

class ArgumentParser {
public:
  explicit ArgumentParser(std::string program_name = {},
                          std::string version = "1.0",
                          default_arguments add_args = default_arguments::all)
      : m_program_name(std::move(program_name)), m_version(std::move(version)) {
    if ((add_args & default_arguments::help) == default_arguments::help) {
      add_argument("-h", "--help")
          .action([&](const auto &/*unused*/) {
            std::cout << help().str();
            std::exit(0);
          })
          .default_value(false)
          .help("shows help message and exits")
          .implicit_value(true)
          .nargs(0);
    }
    if ((add_args & default_arguments::version) == default_arguments::version) {
      add_argument("-v", "--version")
          .action([&](const auto &/*unused*/) {
            std::cout << m_version << std::endl;
            std::exit(0);
          })
          .default_value(false)
          .help("prints version information and exits")
          .implicit_value(true)
          .nargs(0);
    }
  }

  ArgumentParser(ArgumentParser &&) noexcept = default;
  ArgumentParser &operator=(ArgumentParser &&) = default;

  ArgumentParser(const ArgumentParser &other)
      : m_program_name(other.m_program_name),
        m_version(other.m_version),
        m_description(other.m_description),
        m_epilog(other.m_epilog),
        m_is_parsed(other.m_is_parsed),
        m_positional_arguments(other.m_positional_arguments),
        m_optional_arguments(other.m_optional_arguments) {
    for (auto it = std::begin(m_positional_arguments);
         it != std::end(m_positional_arguments); ++it) {
      index_argument(it);
    }
    for (auto it = std::begin(m_optional_arguments);
         it != std::end(m_optional_arguments); ++it) {
      index_argument(it);
    }
  }

  ~ArgumentParser() = default;

  ArgumentParser &operator=(const ArgumentParser &other) {
    auto tmp = other;
    std::swap(*this, tmp);
    return *this;
  }

  // Parameter packing
  // Call add_argument with variadic number of string arguments
  template <typename... Targs> Argument &add_argument(Targs... f_args) {
    using array_of_sv = std::array<std::string_view, sizeof...(Targs)>;
    auto argument = m_optional_arguments.emplace(
        std::cend(m_optional_arguments), array_of_sv{f_args...});

    if (!argument->m_is_optional) {
      m_positional_arguments.splice(std::cend(m_positional_arguments),
                                    m_optional_arguments, argument);
    }

    index_argument(argument);
    return *argument;
  }

  // Parameter packed add_parents method
  // Accepts a variadic number of ArgumentParser objects
  template <typename... Targs>
  ArgumentParser &add_parents(const Targs &...f_args) {
    for (const ArgumentParser &parent_parser : {std::ref(f_args)...}) {
      for (const auto &argument : parent_parser.m_positional_arguments) {
        auto it = m_positional_arguments.insert(
            std::cend(m_positional_arguments), argument);
        index_argument(it);
      }
      for (const auto &argument : parent_parser.m_optional_arguments) {
        auto it = m_optional_arguments.insert(std::cend(m_optional_arguments),
                                              argument);
        index_argument(it);
      }
    }
    return *this;
  }

  ArgumentParser &add_description(std::string description) {
    m_description = std::move(description);
    return *this;
  }

  ArgumentParser &add_epilog(std::string epilog) {
    m_epilog = std::move(epilog);
    return *this;
  }

  /* Call parse_args_internal - which does all the work
   * Then, validate the parsed arguments
   * This variant is used mainly for testing
   * @throws std::runtime_error in case of any invalid argument
   */
  void parse_args(const std::vector<std::string> &arguments) {
    parse_args_internal(arguments);
    // Check if all arguments are parsed
    for ([[maybe_unused]] const auto& [unused, argument] : m_argument_map) {
      argument->validate();
    }
  }

  /* Main entry point for parsing command-line arguments using this
   * ArgumentParser
   * @throws std::runtime_error in case of any invalid argument
   */
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
  void parse_args(int argc, const char *const argv[]) {
    std::vector<std::string> arguments;
    std::copy(argv, argv + argc, std::back_inserter(arguments));
    parse_args(arguments);
  }

  /* Getter for options with default values.
   * @throws std::logic_error if parse_args() has not been previously called
   * @throws std::logic_error if there is no such option
   * @throws std::logic_error if the option has no value
   * @throws std::bad_any_cast if the option is not of type T
   */
  template <typename T = std::string> T get(std::string_view arg_name) const {
    if (!m_is_parsed) {
      throw std::logic_error("Nothing parsed, no arguments are available.");
    }
    return (*this)[arg_name].get<T>();
  }

  /* Getter for options without default values.
   * @pre The option has no default value.
   * @throws std::logic_error if there is no such option
   * @throws std::bad_any_cast if the option is not of type T
   */
  template <typename T = std::string>
  auto present(std::string_view arg_name) const -> std::optional<T> {
    return (*this)[arg_name].present<T>();
  }

  /* Getter that returns true for user-supplied options. Returns false if not
   * user-supplied, even with a default value.
   */
  auto is_used(std::string_view arg_name) const {
    return (*this)[arg_name].m_is_used;
  }

  /* Indexing operator. Return a reference to an Argument object
   * Used in conjuction with Argument.operator== e.g., parser["foo"] == true
   * @throws std::logic_error in case of an invalid argument name
   */
  Argument &operator[](std::string_view arg_name) const {
    auto it = m_argument_map.find(arg_name);
    if (it != m_argument_map.end()) {
      return *(it->second);
    }
    if (arg_name.front() != '-') {
      std::string name(arg_name);
      // "-" + arg_name
      name = "-" + name;
      it = m_argument_map.find(name);
      if (it != m_argument_map.end()) {
        return *(it->second);
      }
      // "--" + arg_name
      name = "-" + name;
      it = m_argument_map.find(name);
      if (it != m_argument_map.end()) {
        return *(it->second);
      }
    }
    throw std::logic_error("No such argument: " + std::string(arg_name));
  }

  // Print help message
  friend auto operator<<(std::ostream &stream, const ArgumentParser &parser)
      -> std::ostream & {
    stream.setf(std::ios_base::left);
    stream << "Usage: " << parser.m_program_name << " [options] ";
    std::size_t longest_arg_length = parser.get_length_of_longest_argument();

    for (const auto &argument : parser.m_positional_arguments) {
      stream << argument.m_names.front() << " ";
    }
    stream << "\n\n";

    if (!parser.m_description.empty()) {
      stream << parser.m_description << "\n\n";
    }

    if (!parser.m_positional_arguments.empty()) {
      stream << "Positional arguments:\n";
    }

    for (const auto &argument : parser.m_positional_arguments) {
      stream.width(longest_arg_length);
      stream << argument;
    }

    if (!parser.m_optional_arguments.empty()) {
      stream << (parser.m_positional_arguments.empty() ? "" : "\n")
             << "Optional arguments:\n";
    }

    for (const auto &argument : parser.m_optional_arguments) {
      stream.width(longest_arg_length);
      stream << argument;
    }

    if (!parser.m_epilog.empty()) {
      stream << parser.m_epilog << "\n\n";
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
  print_help() const {
    auto out = help();
    std::cout << out.rdbuf();
    return out.str();
  }

private:
  /*
   * @throws std::runtime_error in case of any invalid argument
   */
  void parse_args_internal(const std::vector<std::string> &arguments) {
    if (m_program_name.empty() && !arguments.empty()) {
      m_program_name = arguments.front();
    }
    auto end = std::end(arguments);
    auto positional_argument_it = std::begin(m_positional_arguments);
    for (auto it = std::next(std::begin(arguments)); it != end;) {
      const auto &current_argument = *it;
      if (Argument::is_positional(current_argument)) {
        if (positional_argument_it == std::end(m_positional_arguments)) {
          throw std::runtime_error(
              "Maximum number of positional arguments exceeded");
        }
        auto argument = positional_argument_it++;
        it = argument->consume(it, end);
        continue;
      }

      auto arg_map_it = m_argument_map.find(current_argument);
      if (arg_map_it != m_argument_map.end()) {
        auto argument = arg_map_it->second;
        it = argument->consume(std::next(it), end, arg_map_it->first);
      } else if (const auto &compound_arg = current_argument;
                 compound_arg.size() > 1 && compound_arg[0] == '-' &&
                 compound_arg[1] != '-') {
        ++it;
        for (std::size_t j = 1; j < compound_arg.size(); j++) {
          auto hypothetical_arg = std::string{'-', compound_arg[j]};
          auto arg_map_it2 = m_argument_map.find(hypothetical_arg);
          if (arg_map_it2 != m_argument_map.end()) {
            auto argument = arg_map_it2->second;
            it = argument->consume(it, end, arg_map_it2->first);
          } else {
            throw std::runtime_error("Unknown argument: " + current_argument);
          }
        }
      } else {
        throw std::runtime_error("Unknown argument: " + current_argument);
      }
    }
    m_is_parsed = true;
  }

  // Used by print_help.
  std::size_t get_length_of_longest_argument() const {
    if (m_argument_map.empty()) {
      return 0;
    }
    std::size_t max_size = 0;
    for ([[maybe_unused]] const auto& [unused, argument] : m_argument_map) {
      max_size = std::max(max_size, argument->get_arguments_length());
    }
    return max_size;
  }

  using list_iterator = std::list<Argument>::iterator;

  void index_argument(list_iterator it) {
    for (const auto &name : std::as_const(it->m_names)) {
      m_argument_map.insert_or_assign(name, it);
    }
  }

  std::string m_program_name;
  std::string m_version;
  std::string m_description;
  std::string m_epilog;
  bool m_is_parsed = false;
  std::list<Argument> m_positional_arguments;
  std::list<Argument> m_optional_arguments;
  std::map<std::string_view, list_iterator, std::less<>> m_argument_map;
};

} // namespace argparse
