#ifndef __UTILS_CPP__
#define __UTILS_CPP__
#endif

#include "utils.h"

std::random_device rd;
std::mt19937 rnd_gen(rd());

ID_t NUM_PEERS = 0;

void logerr(std::string str)
{
    std::cerr << str << std::endl;
    exit(1);
}

void log(std::string str)
{
    std::cerr << str << std::endl;
    return;
}

std::string ticks2str(Ticks time){
    int secs = time/1000;
    int min = secs / 60;
    secs -= min * 60;
    int milisecs = (time  - secs * 1000);
    return tos(min) + ":" + tos(secs) + "." + tos(milisecs);
}

std::random_device dev;
std::mt19937 rng(dev());