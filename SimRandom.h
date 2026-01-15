#pragma once

#include <random>

/**
 * @brief Random number helper for simulation.
 */
class SimRandom {
public:
    /**
     * @brief Return a random double in the range [0, 1).
     * @return Random double in [0, 1).
     * @note Uses an internal \c std::mt19937 engine.
     */
    static double urand();
};
