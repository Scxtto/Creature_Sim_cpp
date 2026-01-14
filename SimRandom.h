#pragma once

#include <random>

class SimRandom {
public:
    static double random01();
    static int randomInt(int minValue, int maxValue);
};
