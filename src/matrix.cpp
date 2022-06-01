#include "matrix.hpp"
#include "vector.hpp"
#include "util.hpp"
#include <omp.h>
#include <iostream>

// matrix-vector multiplication (optimized with hoisting) and vectorization
template<typename T, typename C>
void matmult(Matrix<T> *m, const Vector<C>& x, Vector<C>& y) {
  #pragma omp simd
  for (size_t i = 0; i < m->num_rows(); ++i) {
    C hoist = 0.0;

    // do cols inner loop since memory aligned
    #pragma omp simd
    for (size_t j = 0; j < m->num_cols(); ++j) {
      hoist += static_cast<C>(m->operator()(i, j)) * x(j);
    }

    // great speed up
    y(i) += hoist;
  }
}

template<typename T, typename C>
C matmultvec(Matrix<T> *m, const size_t row, const Vector<C>& x) {
  C hoist = 0.0;

  // do cols inner loop since memory aligned
  #pragma omp simd
  for (size_t j = 0; j < m->num_cols(); ++j) {
    hoist += static_cast<C>(m->operator()(row, j)) * x(j);
  }

  return hoist;
}


template <typename T>
void Matrix<T>::set_all(const T &val) {
  for(size_t i = 0; i < storage_.size(); i++) {
    storage_.at(i) = val;
  }
}

template <typename T>
void Matrix<T>::zeroize() {
    set_all(0.0);
}

template <typename T>
Vector<T> Matrix<T>::mult(Vector<T>& x) {
  Vector<T> y(num_rows());
  mult(x, y);
  return y;
}

// this is an optimized matrix multiplication
template <typename T>
void Matrix<T>::mult(Vector<T>& x, Vector<T>& y) {
  matmult(this, x, y);
}

template <typename T>
Matrix<short> Matrix<T>::truncshort() {
  std::vector<short> ndata(storage_.size());
  for(size_t i = 0; i < ndata.size(); i++) {
    ndata.at(i) = static_cast<short>(trunc(storage_.at(i)));
  }
  return Matrix<short>(num_rows(), num_cols(), ndata);
}

template <typename T>
Matrix<int> Matrix<T>::truncint() {
  std::vector<int> ndata(storage_.size());
  for(size_t i = 0; i < ndata.size(); i++) {
    ndata.at(i) = static_cast<int>(trunc(storage_.at(i)));
  }
  return Matrix<int>(num_rows(), num_cols(), ndata);
}

template<typename T>
pattern_t Matrix<T>::update(pattern_t &pattern) {
  Vector<double> d_pattern = pattern.convert<double>(); // convert pattern to double
  Vector<double> out(d_pattern.num_rows());

  // apply update inner (more efficient if called directly)
  // requires less copying
  this->update(d_pattern, out);

  // convert back from double to original type
  return out.convert<short>();
}

template<typename T>
void Matrix<T>::update(const Vector<double> in, Vector<double> &out) {
  // apply matrix mult for synchronous update
  out.zeroize();
  matmult<T, double>(this, in, out);

  // now apply update rule for the update vector
  // this won't change types and apply the sign function
  // specifically to the double vector to prevent unnecessary copying
  for (size_t i = 0; i < out.num_rows(); i++) {
    if (dcompare(out(i), 0.0)) {
      out(i) = in(i); // copy previous state
    } else {
      out(i) = vsign<double, double>(out(i)); // get -1/1 sign
    }
  }
}

template<typename T>
size_t Matrix<T>::run_to_min(const pattern_t &pattern, pattern_t &out_pattern) {
  size_t steps = 0;

  // calculate initial energy
  double energy = this->energy(pattern);

  // convert to doubles
  Vector<double> o_pattern = pattern.convert<double>();

  // create vector of indices (from original pattern)
  std::vector<size_t> indx(pattern.num_rows());
  for (size_t i = 0; i < indx.size(); i++) {
    indx[i] = i;
  }

  std::random_device rand_dev;
  std::mt19937 generator(rand_dev());
  std::uniform_int_distribution<size_t> distr(0, indx.size() - 1);

  // only continue to update while the energy is changing
  size_t not_changed = 0;
  while (true) {
    // randomly shuffle indices
    std::shuffle(indx.begin(), indx.end(), generator);

    // increase steps
    steps++;

    // go through all neurons/states and apply threshold
    for (size_t i = 0; i < indx.size(); i++) {
      size_t ind = indx[i];  // distr(generator);
      double nvalue = matmultvec(this, ind, o_pattern); // apply threshold function
      if(nvalue > EPS) {  // update value only if it has changed
        o_pattern(ind) = 1.0;
      } else if (nvalue < EPS) {
        o_pattern(ind) = -1.0;
      }
    }
    double nenergy = this->energy(o_pattern);

    // check if energy has changed
    if (dcompare(energy, nenergy)) {
      not_changed++;
      if (not_changed >= 5) {
        break;
      }
    } else {
      not_changed = 0;
    }
    
    // copy new energy
    energy = nenergy; 
  }

  // copy back to output
  for (size_t i = 0; i < num_rows(); i++) {
    out_pattern(i) = static_cast<short>(floor(o_pattern(i)));
  }

  return steps;
}

template<typename T>
void Matrix<T>::train_on(patterns_t &patterns) {
  const double pattern_n = static_cast<double>(patterns.size());
  
  // train on every pattern
  for (size_t p = 0; p < patterns.size(); ++p) {
    Vector<double> patt = patterns.at(p).convert<double>(); // cache pattern (converted to doubles)
    for (size_t i = 0; i < num_rows(); ++i) {
      double ival = patt(i); // cache value
      for (size_t j = 0; j < num_cols(); ++j) {
        double val = ival*patt(j); // Hebb's rule
        storage_[i * num_cols_ + j] += val; // setting w_ij
        storage_[j * num_cols_ + i] += val; // setting w_ji
      }
    }
  }

  // divide valus by p (for Hebb's rule)
  for (size_t i = 0; i < storage_.size(); i++) {
    storage_[i] /= pattern_n;
  }

  // zero out diagonal
  for (size_t i = 0; i < num_rows(); i++) {
    storage_[i * num_cols_ + i] = 0.0;
  }
}

hopfield_pt train_hopfield(patterns_t &patterns) {
  if (patterns.empty()) {
    std::cerr << "Empty pattern list" << std::endl;
    std::exit(1);
    // return new Matrix<double>(1, 1);
  }

  // determine the size of a pattern
  pattern_t &patt = patterns.at(0);
  size_t N = patt.num_rows();

  // construct the Matrix
  hopfield_pt hopfield = new Matrix<double>(N, N);
  hopfield->zeroize();
  hopfield->train_on(patterns);
  
  return hopfield;
}

void delete_hopfield(hopfield_pt ptr) {
  if (ptr == NULL) return;
  delete ptr;
}

// force declarations of the following templates
template class Matrix<int>;
template class Matrix<double>;
template class Matrix<short>;
