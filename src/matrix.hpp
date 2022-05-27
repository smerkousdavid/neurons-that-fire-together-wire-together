#ifndef MATRIX_HPP
#define MATRIX_HPP

#include <cstddef>
#include <vector>

template<typename T>
class Matrix {
public:
  Matrix(size_t M, size_t N) : num_rows_(M), num_cols_(N), storage_(num_rows_ * num_cols_) {}

        T& operator()(size_t i, size_t j)       { return storage_[i * num_cols_ + j]; }
  const T& operator()(size_t i, size_t j) const { return storage_[i * num_cols_ + j]; }

  void set_all(const T &);
  void zeroize();

  size_t num_rows() const { return num_rows_; }
  size_t num_cols() const { return num_cols_; }

private:
  size_t              num_rows_, num_cols_;
  std::vector<T> storage_;
};


#endif