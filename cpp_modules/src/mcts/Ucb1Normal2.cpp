#include <iostream>
#include "Ucb1Normal2.h"

// Constructor which allows us to choose numArms of arms
UCB1Normal2::UCB1Normal2(int numArms) : totalTrials(0) {
  // Debug for numArms
  if(numArms <= 0) {
    std::cerr << "Error: numArms must be greater than 0, received: " << numArms << std::endl;
      //throw std::invalid_argument("numArms");
  }
  rewards.resize(numArms);
  std::cout << "Initialized UCB1Normal2 with " << numArms << " arms." << std::endl;
}

void UCB1Normal2::addReward(int arm, double reward) {
    // debug
    if (arm < 0 || arm >= rewards.size()) {
      std::cout << "Error: arm index " << arm << " out of bounds" << std::endl;
      throw::std::out_of_range("UCB1Normal2::addReward");
    }

    std::cout << "Adding reward for arm " << arm << " with value " << reward << std::endl;
    rewards[arm].push_back(reward);
    totalTrials++;
}

// Selecting the best arm based on the Ucb1-Normal2 
int UCB1Normal2::selectArm() {
    double maxUCB = -std::numeric_limits<double>::infinity();
    int selectedArm = -1;

    // Ensure that we always explore the unexplored arms first
    for (size_t i = 0; i < rewards.size(); ++i) {
      // Debug test for arm index and rewards size
      if (i >= rewards.size()) {
        std::cout << "Error: Accessing invalid arm index " << i << " in rewards vector" << std::endl;
      }

        if (rewards[i].empty()) {
          //Debug test
          std::cout << "Arm" << i << " has no rewards. Selecting for exploration" << std::endl;
            return i;
        }

        double mean = calculateMean(rewards[i]);
        double variance = calculateVariance(rewards[i], mean);
        int trials = rewards[i].size();
        // This is the new and different form of the equation we had for UCT
        double ucb = mean + std::sqrt(2 * std::log(totalTrials) / trials) * std::sqrt(variance);

        if (ucb > maxUCB) {
            maxUCB = ucb;
            selectedArm = i;
        }
    }

    return selectedArm;
}

// Calculate the mean of the given samples
double UCB1Normal2::calculateMean(const std::vector<double>& samples) {
    if (samples.empty()) return 0.0;
    return std::accumulate(samples.begin(), samples.end(), 0.0) / samples.size();
}

// Calculate the variance of the given samples, within the mean
double UCB1Normal2::calculateVariance(const std::vector<double>& samples, double mean) {
    if (samples.size() < 2) return 0.0;
    double variance = 0.0;
    for (double sample : samples) {
        variance += (sample - mean) * (sample - mean);
    }
    return variance / samples.size();
}
