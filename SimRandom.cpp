#include "SimRandom.h"

static std::mt19937& rng()
{
    static std::mt19937 engine(std::random_device{}());
    return engine;
}

double SimRandom::random01()
{
    static std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(rng());
}

int SimRandom::randomInt(int minValue, int maxValue)
{
    std::uniform_int_distribution<int> dist(minValue, maxValue);
    return dist(rng());
}
