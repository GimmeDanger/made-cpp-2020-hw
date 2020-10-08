#pragma once

#include <algorithm> //< for std::transform, std::copy, std::iter_swap
#include <cassert>   //< for assert
#include <cmath>     //< for fabs
#include <numeric>   //< for std::accumulate
#include <optional>  //< for std::optional
#include <utility>   //< for std::plus, std::minus, std::multiplies
#include <vector>    //< for std::vector

namespace task {

// Transform wrapper for vectors
template <typename T, typename Functor>
std::vector<T> VectorTransform(const std::vector<T> &lhs,
                                const std::vector<T> &rhs, Functor func) {
  assert(lhs.size() == rhs.size());
  std::vector<T> out(lhs.size());
  std::transform(cbegin(lhs), cend(lhs), cbegin(rhs), begin(out), func);
  return out;
}

/**
 *  Binary operator plus
 *  res[i] = lhs[i] + rhs[i]
 */
template <typename T>
std::vector<T> operator+(const std::vector<T> &lhs, const std::vector<T> &rhs) {
  return VectorTransform(lhs, rhs, std::plus<>{});
}

/**
 *  Binary operator minus
 *  res[i] = lhs[i] - rhs[i]
 */
template <typename T>
std::vector<T> operator-(const std::vector<T> &lhs, const std::vector<T> &rhs) {
  return VectorTransform(lhs, rhs, std::minus<>{});
}

/**
 *  Unary operator plus
 *  res[i] = (+1) * v[i]
 */
template <typename T>
std::vector<T> operator+(const std::vector<T> &v) {
  return v;
}

/**
 *  Unary operator minus
 *  res[i] = (-1) * v[i]
 */
template <typename T>
std::vector<T> operator-(const std::vector<T> &v) {
  std::vector<T> out(v.size());
  std::transform(cbegin(v), cend(v), begin(out),
                 [](const T &a) { return -a; });
  return out;
}

/**
 *  Binary operator of scalar product,
 *  res = sum_i(lhs_i * rhs_i)
 */
template <typename T>
double operator*(const std::vector<T> &lhs, const std::vector<T> &rhs) {
  assert(lhs.size() == rhs.size());
  double res = 0.0;
  for (size_t i = 0; i < lhs.size(); i++)
    res += lhs[i] * rhs[i];
  return res;
}

/**
 *  Binary operator of cross product,
 *  correctly determined only for n=3,
 *  check wiki for details
 */
template <typename T>
std::vector<T> operator%(const std::vector<T> &lhs, const std::vector<T> &rhs) {
  constexpr size_t n = 3;
  constexpr size_t x = 0;
  constexpr size_t y = 1;
  constexpr size_t z = 2;
  assert(lhs.size() == n && rhs.size() == n);
  return {lhs[y] * rhs[z] - lhs[z] * rhs[y],
          lhs[z] * rhs[x] - lhs[x] * rhs[z],
          lhs[x] * rhs[y] - lhs[y] * rhs[x]};
}

/**
 * Collinearity check helper, find alpha such that @lhs = alpha * @rhs,
 * otherwise returns empty optional, if vectors are not collinear
 */
template <typename T>
std::optional<double> CollinearityMult(const std::vector<T> &lhs, const std::vector<T> &rhs) {

  // required for in_nonzero(double) -> bool helper
  static constexpr double nonzero_check_eps = 1.e-64;
  // required for collinearity check algorithm
  static constexpr double ratio_diff_check_eps = 1.e-7;

  assert(lhs.size() == rhs.size());
  auto is_nonzero = [](double value) {
    return fabs(value) > nonzero_check_eps;
  };

  // corner case: zero vector is collinear with any other
  if (   lhs.empty() || !is_nonzero(lhs * lhs)
      || rhs.empty() || !is_nonzero(rhs * rhs))
    return {0.};

  // otherwise, collinear vectors must have equal ratio factor > 0
  auto nonzero_begin = std::find_if(begin(lhs), end(rhs), is_nonzero);
  size_t nonzero_begin_pos = std::distance(begin(lhs), nonzero_begin);
  if (nonzero_begin == end(lhs) || !is_nonzero(rhs[nonzero_begin_pos]))
    return {};

  const auto candidate_ratio = lhs[nonzero_begin_pos] / rhs[nonzero_begin_pos];

  for (size_t i = nonzero_begin_pos + 1; i < lhs.size(); i++) {
    if ((!is_nonzero(lhs[i]) && is_nonzero(rhs[i])) ||
        (is_nonzero(lhs[i]) && !is_nonzero(rhs[i]))) {
      return {};
    }
    else if (is_nonzero(lhs[i]) && is_nonzero(rhs[i])) {
      double diff = candidate_ratio - lhs[i] / rhs[i];
      // collinear vectors must have equal ratio factor
      if (fabs(diff) > ratio_diff_check_eps)
        return {};
    }
  }

  return {candidate_ratio};
}

/**
 * Binary operator of collinearity check,
 * implementation is encapsulated in collinearity mult getter
 */
template <typename T>
bool operator||(const std::vector<T> &lhs, const std::vector<T> &rhs) {
  return CollinearityMult(lhs, rhs).has_value();
}

/**
 * Binary operator of same direction check,
 * true if collinear vectors have same direction,
 * false otherwise
 */
template <typename T>
bool operator&&(const std::vector<T> &lhs, const std::vector<T> &rhs) {
  auto opt = CollinearityMult(lhs, rhs);
  return opt.has_value() && opt.value() > 0.;
}

/**
 * Binary operator of bitwise |, implemented only for integer types
 */
template <typename T>
std::vector<T> operator|(const std::vector<T> &lhs, const std::vector<T> &rhs) {
  static_assert(std::is_integral<T>::value, "Integral required.");
  return VectorTransform(lhs, rhs, std::bit_or<>{});
}

/**
 * Binary operator of bitwise &, implemented only for integer types
 */
template <typename T>
std::vector<T> operator&(const std::vector<T> &lhs, const std::vector<T> &rhs) {
  static_assert(std::is_integral<T>::value, "Integral required.");
  return VectorTransform(lhs, rhs, std::bit_and<>{});
}

/**
 * Input operator >>, first read number is a size of @v, other values are
 * vector's elements
 */
template <typename T>
std::istream& operator>>(std::istream &s, std::vector<T> &v) {
  size_t len;
  s >> len;
  v.resize(len);
  for (size_t i = 0; i < len; i++)
    s >> v[i];
//  std::istream_iterator<T> stream_begin(s), stream_end;
//  std::copy(stream_begin, stream_end, begin(v));
  return s;
}

/**
 * Output operator <<, copy all vector values into stream @s with space as
 * delimiter, output is ending with newline
 */
template <typename T>
std::ostream& operator<<(std::ostream &s, const std::vector<T> &v) {
  for (size_t i = 0; i < v.size(); i++)
    s << v[i] << " ";
  // std::copy(begin(v), end(v), std::ostream_iterator<T>(s, " "));
  s << std::endl;
  return s;
}

/**
 * Reverse elements of @v
 */
template <typename T>
void reverse(std::vector<T> &v) {
  // impl of std::reverse(begin(v), end(v));
  auto first = v.begin(), last = v.end();
  while ((first != last) && (first != --last)) {
    std::iter_swap(first++, last);
  }
}

} // namespace task

