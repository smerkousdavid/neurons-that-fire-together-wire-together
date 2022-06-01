#ifndef MATRIX_HPP
#define MATRIX_HPP

#include <cstddef>
#include <vector>
#include <cmath>
#include <type_traits>
#include <iostream>
#include <tuple>
#include <omp.h>
#include "vector.hpp"
#include "util.hpp"

template<typename T>
class Matrix {
public:
  Matrix(size_t N) : num_rows_(N), num_cols_(N), storage_(num_rows_ * num_cols_) {}
  Matrix(size_t M, size_t N) : num_rows_(M), num_cols_(N), storage_(num_rows_ * num_cols_) {}
  Matrix(size_t M, size_t N, const std::vector<T> &W) : num_rows_(M), num_cols_(N), storage_(W) {}

        T& operator()(size_t i, size_t j)       { return storage_[i * num_cols_ + j]; }
  const T& operator()(size_t i, size_t j) const { return storage_[i * num_cols_ + j]; }

  void set_all(const T &);
  void zeroize();

  void mult(Vector<T> &x, Vector<T> &y);
  Vector<T> mult(Vector<T> &x);

  template<typename C>
  Matrix<C> convert() const {
    // if (std::is_same<T, C>::value) {
    //   return this; // return this object (don't do conversion)
    // }

    std::vector<C> ndata(storage_.size());
    for(size_t i = 0; i < ndata.size(); i++) {
      ndata.at(i) = static_cast<C>(storage_.at(i));
    }
    return Matrix<C>(num_rows(), num_cols(), ndata);
  }

  Matrix<short> truncshort();
  Matrix<int> truncint();

  template<typename C=short>
  Matrix<C> sign() {
    std::vector<C> ndata(storage_.size());
    for(size_t i = 0; i < ndata.size(); i++) {
      ndata.at(i) = vsign<T, C>(storage_.at(i));
    }
    return Matrix<C>(num_rows(), num_cols(), ndata);
  }

  template<typename C>
  double energy(const Vector<C> &pattern) {
    double e = 0.0;

    for (size_t i = 0; i < num_rows(); i++) {
      double vc = static_cast<double>(pattern(i)); // cache current s_i
      for (size_t j = 0; j < num_cols(); j++) {
        e -= (static_cast<double>(storage_[i * num_cols_ + j]) * vc * static_cast<double>(pattern(j)));  // this is w_ij * si * sj (inline with memory j)
      } 
    }

    return 0.5 * e;
  }

  void train_on(std::vector<Vector<short>> &patterns);
  pattern_t update(pattern_t &pattern);
  void update(const Vector<double> in, Vector<double> &out);
  size_t run_to_min(const pattern_t &pattern, pattern_t &out_pattern);

  void print() {
    for (size_t i = 0; i < num_rows(); i++) {
      std::cout << "[";
      for (size_t j = 0; j < num_cols(); j++) {
        std::cout << this->operator()(i, j) << ((j == (num_cols() - 1)) ? "" : " ");
      }
      std::cout << "]" << std::endl;
    }
  }

  size_t num_rows() const { return num_rows_; }
  size_t num_cols() const { return num_cols_; }

private:
  size_t              num_rows_, num_cols_;
  std::vector<T> storage_;
};

typedef Matrix<double> hopfield_t;
typedef Matrix<double>* hopfield_pt;

// define simple functions to create/destroy hopfields
hopfield_pt train_hopfield(patterns_t &patterns);
void delete_hopfield(hopfield_pt ptr);

#endif