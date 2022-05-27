#ifndef VECTOR_HPP
#define VECTOR_HPP

#include <cstddef>
#include <vector>

template <typename T>
class Vector {
public:
  Vector(size_t M) : num_rows_(M), storage_(num_rows_) {}

        T& operator()(size_t i)       { return storage_[i]; }
  const T& operator()(size_t i) const { return storage_[i]; }

  void set_all(const T &);
  void zeroize();

  size_t num_rows() const { return num_rows_; }

private:
  size_t              num_rows_;
  std::vector<T> storage_;
};

#endif