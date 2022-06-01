#ifndef UTIL_HPP
#define UTIL_HPP

#include <random>
#include <vector>
#include <cmath>

#define EPS 0.0001 // define arbitrary epsilon to compare doubles

// fast sign function https://stackoverflow.com/questions/14579920/fast-sign-of-integer-in-c
template <typename T, typename C>
C inline vsign(const T &x) {
  return static_cast<C>((x > 0) - (x < 0));
}

// determine if any within
template <typename T>
bool inline dcompare(const T &f, const T &s) {
  return f == s;
}

// determine if double is within eps
template <>
bool inline dcompare(const double &f, const double &s) {
  return std::abs(f - s) <= EPS;
}

// random function based off of https://stackoverflow.com/questions/288739/generate-random-numbers-uniformly-over-an-entire-range
template <typename T>
T random_uniform(const T &range_from, const T &range_to) {
  std::random_device rand_dev;
  std::mt19937 generator(rand_dev());
  std::uniform_real_distribution<T> distr(range_from, range_to);
  return distr(generator);
}

template <typename T>
std::vector<T> random_uniform_vector(const size_t num, const double &range_from, const double &range_to) {
  std::random_device rand_dev;
  std::mt19937 generator(rand_dev());
  std::uniform_real_distribution<double> distr(range_from, range_to);

  std::vector<T> vals(num);
  for (size_t i = 0; i < vals.size(); i++) {
    vals[i] = static_cast<T>(distr(generator));
  }
  return vals;
}

// std::vector<size_t> random_index_vector(size_t num) {
//   // create vector of indices (from original pattern)
//   std::vector<size_t> indx(orig.num_rows());
//   for (size_t i = 0; i < indx.size(); i++) {
//     indx[i] = i;
//   }

//   // randomly shuffle indices
//   std::random_device rand_dev;
//   std::mt19937 generator(rand_dev());
//   std::shuffle(indx.begin(), indx.end(), generator);
//   size_t dist = static_cast<size_t>(distance);
// }

#endif