#include "SimRandom.h"
#include <random>

static thread_local std::mt19937 rng{std::random_device{}()};

double SimRandom::urand()
{    
    static thread_local std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(rng);
}