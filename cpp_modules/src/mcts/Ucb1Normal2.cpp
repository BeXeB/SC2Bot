#include "Ucb1Normal2.h"

// Constructor which allows us to choose numArms of arms
UCB1Normal2::UCB1Normal2(int numArms) : totalTrials(0) {
    rewards.resize(numArms);
}

void UCB1Normal2::addReward(int arm, double reward) {
    rewards[arm].push_back(reward);
    totalTrials++;
}

// Selecting the best arm based on the Ucb1-Normal2 
int UCB1Normal2::selectArm() {
    double maxUCB = -std::numeric_limits<double>::infinity();
    int selectedArm = -1;

    // Ensure that we always explore the unexplored arms first
    for (size_t i = 0; i < rewards.size(); ++i) {
        if (rewards[i].empty()) {
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
