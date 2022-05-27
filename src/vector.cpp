#include "vector.hpp"

template <typename T>
void Vector<T>::set_all(const T &val) {
  for(size_t i = 0; i < num_rows(); i++) {
    storage_.at(i) = val;
  }
}

template <typename T>
void Vector<T>::zeroize() {
    set_all(0.0);
}

// force declarations of the following templates
template class Vector<int>;
template class Vector<double>;
template class Vector<bool>;
template class Vector<short>;