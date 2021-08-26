#include "utils.h"

std::random_device rd;
std::mt19937 rnd_gen(rd());

double _exp(double mean)
{
    if(mean == 0) return 0;
    std::exponential_distribution< > rng(1 / mean);
    return rng(rnd_gen);
}