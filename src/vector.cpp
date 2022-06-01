#include "vector.hpp"
#include "util.hpp"
#include <type_traits>

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

template <typename T>
void Vector<T>::randomize(const T &range_from, const T &range_to) {
  storage_ = random_uniform_vector<T>(storage_.size(), range_from, range_to);
}

template<typename T>
void Vector<T>::randomize() {
    std::random_device rand_dev;
    std::mt19937 generator(rand_dev());
    std::uniform_int_distribution<short> distr(0, 1); 

    // uniform [0, 1] for pattern
    if (std::is_same<T, int>::value || std::is_same<T, short>::value) {
      for (size_t i = 0; i < num_rows(); i++) {
        storage_[i] = (distr(generator) * 2) - 1; // map to -1 or 1
      }
    } else {
      for (size_t i = 0; i < num_rows(); i++) {
        storage_[i] = static_cast<T>((distr(generator) * 2.0) - 1.0); // map to -1 or 1
      }
    }
}

// template <typename T>
// Vector<T> Vector<T>::hammed_random(const size_t distance) {
//   return null;
// }

template<typename T>
size_t Vector<T>::hamming(Vector<T> &other) {
  size_t diff = 0;
  for (size_t i = 0; i < num_rows(); i++) {
    if (storage_[i] != other(i)) {
      diff++;
    }
  }
  return diff;
}


void make_random_patterns(const size_t neurons, patterns_t &patts, const size_t num) {
    // create random patterns
    for (size_t i = 0; i < num; i++) {
      pattern_t npattern(neurons);
      npattern.randomize();
      patts.push_back(npattern);
    }
}

void make_hammed_patterns(const pattern_t &orig, patterns_t &patts, const size_t num, const short distance, bool incremental) {
    // create vector of indices (from original pattern)
    std::vector<size_t> indx(orig.num_rows());
    for (size_t i = 0; i < indx.size(); i++) {
      indx[i] = i;
    }
    
    // randomly shuffle indices
    std::random_device rand_dev;
    std::mt19937 generator(rand_dev());
    std::shuffle(indx.begin(), indx.end(), generator);
    size_t dist = static_cast<size_t>(distance);

    // for each new pattern randomly change by the distance
    for (size_t i = 0; i < num; i++) {
      // copy original (or if incremental then last one)
      pattern_t patt = (incremental) ? (patts.at(i).copy()) : orig.copy();

      // shift through indices
      for (size_t j = (i*dist); j < ((i + 1)*dist); j++) {
        size_t id = indx[j % orig.num_rows()]; // get random indx
        patt(id) = -1*patt(id);  // flip a random index
      }

      // add the new pattern
      patts.push_back(patt);
    }
}

// force declarations of the following templates
template class Vector<int>;
template class Vector<double>;
template class Vector<short>;