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
    static double random01();
    /**
     * @brief Return a random integer in the inclusive range [minValue, maxValue].
     * @param minValue Lower bound (inclusive).
     * @param maxValue Upper bound (inclusive).
     * @return Random integer in [minValue, maxValue].
     * @note Uses an internal \c std::mt19937 engine.
     */
    static int randomInt(int minValue, int maxValue);
};
