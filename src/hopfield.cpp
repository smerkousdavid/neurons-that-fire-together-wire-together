#include "matrix.hpp"
#include "vector.hpp"
#include <iostream>
#include <fstream>
#include <string.h>
#include <cmath>
#include <omp.h>

// general flag to see progress of network specifically
// #define DEBUG

// how many neurons to use for proportion convergence simulation
#define PROPORTION_NEURONS_MIN 25
#define PROPORTION_NEURONS_MAX 450
#define PROPORTION_NEURONS_STEP 25

// how many patterns to train on (starts with 0 extra as in only the original pattern)
#define PROPORTION_TRAIN_PATTERNS_MAX static_cast<size_t>(std::ceil(static_cast<double>(num_neurons) / (std::sqrt(2.0 * std::log(static_cast<double>(num_neurons)))))) // absolute max for Hebb which is N / sqrt(2 * ln(N))
#define PROPORTION_TRAIN_PATTERNS_STEP 3 // how many patterns to increment by

// hamming from original pattern (starts at 1) of the test patterns to test convergence on 
#define PROPORTION_RUN_PATTERN_HAMMING_MAX num_neurons * 0.5 // how far away to generate the maximum hamming patternt to test convergence (this value is proportion of number of neurons)
#define PROPORTION_RUN_PATTERN_HAMMING_STEP 3 // how much to increase hamming each time

// how many patterns and simulations to test on
#define PROPORTION_RUN_PATTERNS 100 // how many patterns to test the proportion on
#define PROPORTION_SIMULATION_PER_STEP 125  // run XX simulations for each step


int proportion_of_convergence(hopfield_t &hopfield, const size_t num_patterns, const size_t hamming, const bool train_hammed, const size_t train_hamming, const size_t num_train_patterns) {
  // runs a simulation calculating the proportion of valus converging in parallel

  // create the original pattern
  #ifdef DEBUG
    std::cout << "Creating random pattern" << std::endl;
  #endif
  const size_t neuron_size = hopfield.num_rows();
  pattern_t pattern(neuron_size);
  pattern.randomize(); // give 1/2 prob to each 1,-1

  // container for all patterns (original pattern will be included in this)
  {
    patterns_t train_patterns;

    // add original pattern to training
    train_patterns.push_back(pattern);

    // now create the mostly orthoginal (until a certain point) patterns that are all a certain hamming away
    if (train_hammed) { // let's specify the hamming distance for our training patterns? would be interesting to see radius of convergence as ham distance changes
      #ifdef DEBUG
        std::cout << "Creating training patterns " << num_train_patterns << " patterns with radius " << train_hamming << " from original" << std::endl;
      #endif
      make_hammed_patterns(pattern, train_patterns, num_train_patterns, train_hamming, false); // last flag means increment hamming which we want to be false
    } else {
      #ifdef DEBUG
        std::cout << "Creating random training patterns " << num_train_patterns << std::endl;
      #endif
      // add random patterns to training
      make_random_patterns(neuron_size, train_patterns, num_train_patterns);
    }

    // train a network on the specified patterns
    #ifdef DEBUG
      std::cout << "Training network" << std::endl;
    #endif

    // train the hopfield network
    hopfield.zeroize();  // zeroize weights to prevent additional adding
    hopfield.train_on(train_patterns);
  } // on exit scope train patterns should be deleted to free memory

  #ifdef DEBUG
    std::cout << "Creating " << num_patterns << " hammed patterns from original with radius " << hamming << std::endl;
  #endif
  patterns_t patterns;
  make_hammed_patterns(pattern, patterns, num_patterns, hamming, false); // make it not incremementla

  // keep track of proportions
  int converged = 0.0;
  for (pattern_t &ref_pattern : patterns) {
    pattern_t retrieved_memory(neuron_size);

    // run until we reach an energy minimum
    hopfield.run_to_min(ref_pattern, retrieved_memory);

    // is the retrieved memory from the hopfield network similar to our hammed distance one?
    if (retrieved_memory.similar(pattern)) {
        converged++; // add one to converged
    }
  }

  // return the proportion that have converged
  return converged;
}

void run_proportion_simulations() {
  // run various monte carlo simulations to determine behaviour of various convergence (number of patterns)

  // create the csv file
  std::ofstream data("proportion-data.csv");
  data << "neurons,trained_patterns,test_patterns,test_pattern_hamming,simulations_per_step,min_proportion,mean_proportion,max_proportion,std_proportion,25_perc,mode,75_perc" << std::endl;

  for (size_t num_neurons = PROPORTION_NEURONS_MIN; num_neurons < PROPORTION_NEURONS_MAX; num_neurons += PROPORTION_NEURONS_STEP) {
    for (size_t train_patterns = 0; train_patterns < PROPORTION_TRAIN_PATTERNS_MAX; train_patterns += PROPORTION_TRAIN_PATTERNS_STEP) {
      std::cout << "---- Running simulations on " << num_neurons << " neurons and " << train_patterns << " trained patterns ----" << std::endl;

      // calculate the maximum hamming
      size_t max_hamming = static_cast<size_t>(PROPORTION_RUN_PATTERN_HAMMING_MAX) + 1;

      #pragma omp parallel for // multithread the task
      for (size_t hamming = 1; hamming < max_hamming; hamming += PROPORTION_RUN_PATTERN_HAMMING_STEP) {
        // reuse network to reduce copying/zeroing
        hopfield_t hopfield(num_neurons, num_neurons);

        // get basic stats
        int min = INT32_MAX;
        int max = -INT32_MAX;
        double mean = 0.0;
        double vals[PROPORTION_SIMULATION_PER_STEP];
        for (size_t j = 0; j < PROPORTION_SIMULATION_PER_STEP; j++) {
          int prop = proportion_of_convergence(hopfield, PROPORTION_RUN_PATTERNS, hamming, false, 0, train_patterns);
          if (prop < min) {
            min = prop;
          }
          if (prop > max) {
            max = prop;
          }
          mean += static_cast<double>(prop);
          vals[j] = prop;
        }

        // get stats
        mean /= static_cast<double>(PROPORTION_SIMULATION_PER_STEP);
        
        // calculate standard deviation
        double sum = 0.0;
        for (size_t j = 0; j < PROPORTION_SIMULATION_PER_STEP; j++) {
          sum += (vals[j] - mean)*(vals[j] - mean);
        }
        double std = std::sqrt(sum / static_cast<double>(PROPORTION_SIMULATION_PER_STEP - 1));

        // get percentiles
        std::sort(vals, vals + PROPORTION_SIMULATION_PER_STEP);
        double twentyfive = vals[static_cast<size_t>(0.25 * PROPORTION_SIMULATION_PER_STEP)];
        double mode = vals[static_cast<size_t>(0.50 * PROPORTION_SIMULATION_PER_STEP)];
        double seventyfive = vals[static_cast<size_t>(0.75 * PROPORTION_SIMULATION_PER_STEP)];

        #pragma omp critical
        {
          // save results to CSV file
          data << num_neurons << "," << train_patterns << "," << PROPORTION_RUN_PATTERNS << "," << hamming << "," << PROPORTION_SIMULATION_PER_STEP << "," << min 
              << "," << mean << "," << max << "," << std << "," << twentyfive << "," << mode << "," << seventyfive << std::endl;
        }
      }
    }
  }

  // close the file
  data.close();
}


int main(int argc, char* argv[]) {
  std::cout << "Running proportion simulations" << std::endl;
  run_proportion_simulations();

  /*

  patterns_t patts;
  patts.push_back(Vector<short>(5, {-1, 1, 1, 1, -1})); 
  patts.push_back(Vector<short>(5, {1, -1, -1, -1, 1})); 

  hopfield_pt hop = train_hopfield(patts);

  // print contents
  hop->print();

  pattern_t memory(5);
  patterns_t newp;
  make_hammed_patterns(Vector<short>(5, {-1, 1, 1, 1, -1}), newp, 5, 3, false);
  // hop->run_to_min(Vector<short>(5, {-1, 1, 1, 1, -1}), memory);

  for (pattern_t p : newp) {
      memory.zeroize();
      hop->run_to_min(p, memory);
      memory.print();
  }
  // memory.print();

  delete_hopfield(hop);
  */

  std::cout << "Finished running!" << std::endl;
  // create a random pattern
  // std::cout << "orig" << std::endl;
  // pattern_t pattern(5, {1, 1, -1, 1, 1});
  // // pattern.randomize();
  // // pattern.set_all(-1);
  // // for (size_t i = 0; i < pattern.num_rows(); i++) {
  // //     if ((i % 300) < 150) {
  // //         pattern(i) = 1;
  // //     } else {
  // //         pattern(i) = -1;
  // //     }
  // // }

  // // now create patterns that are similar
  // // std::cout << "orig2" << std::endl;
  // patterns_t patterns;
  // patterns.push_back(pattern);
  // // make_hammed_patterns(pattern, patterns, 1, 100, true);

  // // train the hopfield
  // // std::cout << "orig3" << std::endl;
  // hopfield_pt hopfield = train_hopfield(patterns);

  // // get energy of pattern
  // // std::cout << "orig4" << std::endl;

  // std::cout << hopfield->energy(pattern) << std::endl;

  // // now keep running
  // pattern_t memory(pattern.num_rows());
  // size_t steps = hopfield->run_to_min(Vector<short>(5, {-1, -1, 1, 1, -1}), memory);

  // std::cout << hopfield->energy(memory) << " " << memory.similar(pattern) << " steps " << steps << " stuff ";
  // for(int i=0; i < memory.num_rows(); i++) std::cout << memory(i) << ' ';
  // std::cout << std::endl;

  // delete_hopfield(hopfield);
  return 0;
}