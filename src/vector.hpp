#ifndef VECTOR_HPP
#define VECTOR_HPP

#include <cstddef>
#include <vector>
#include <type_traits>
#include <iostream>
#include <algorithm>
#include "util.hpp"

template <typename T>
class Vector {
public:
  Vector(size_t M) : num_rows_(M), storage_(num_rows_) {}
  Vector(size_t M, const std::vector<T> &C) : num_rows_(M), storage_(C) {}

        T& operator()(size_t i)       { return storage_[i]; }
  const T& operator()(size_t i) const { return storage_[i]; }

  void set_all(const T &);
  void zeroize();
  void randomize(const T &, const T &);
  void randomize();
  Vector<T> hammed_random(const T &, const T &, const size_t); 

  template<typename C=short>
  Vector<C> sign() const {
    Vector<C> cp(num_rows());
    for(size_t i = 0; i < num_rows(); i++) {
      cp(i) = vsign<T, C>(storage_.at(i));
    }
    return cp;
  }

  Vector<T> copy() const {
    return Vector<T>(num_rows(), storage_);
  }

  void copy_from(Vector<T> &other) {
    for (size_t i = 0; i < other.num_rows(); i++) {
      storage_[i] = other(i);
    }
  }

  bool similar(const Vector<T> &other) const {
    for (size_t i = 0; i < other.num_rows(); i++) {
      if (!dcompare(storage_[i], other(i))) {
        return false;
      }
    }
    return true;
  }

  template<typename C>
  Vector<C> convert() const {
    // if (std::is_same<T, C>::value) {
    //   return *this; // return this object (don't do conversion)
    // }

    std::vector<C> ndata(storage_.size());
    for(size_t i = 0; i < ndata.size(); i++) {
      ndata.at(i) = static_cast<C>(storage_.at(i));
    }
    return Vector<C>(num_rows(), ndata);
  }

  void print() {
    std::cout << "[";
    for (size_t i = 0; i < num_rows(); i++) {
      std::cout << this->operator()(i) << ((i == (num_rows() - 1)) ? "" : " ");
    }
    std::cout << "]" << std::endl;
  }

  size_t hamming(Vector<T> &);
  // Vector<T> hammed_random(const size_t distance);
  size_t num_rows() const { return num_rows_; }

private:
  size_t              num_rows_;
  std::vector<T> storage_;
};

typedef Vector<short> pattern_t;
typedef std::vector<Vector<short>> patterns_t;
typedef std::vector<Vector<short>>* patterns_pt;

void make_random_patterns(const size_t neurons, patterns_t &patts, const size_t num);
void make_hammed_patterns(const pattern_t &orig, patterns_t &patts, const size_t num, const short distance, bool incremental);
void delete_patterns(patterns_pt patts);

#endif