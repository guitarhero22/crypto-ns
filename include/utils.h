#ifndef __UTILS_H__
#define __UTILS_H__

#include<random>
#include<chrono>
#include<iostream>

typedef unsigned long ID_t; // ID for peers
typedef unsigned long BID_t; // ID for Blocks
typedef unsigned long TID_t; // ID for Txns
typedef unsigned int coin_t; // For Number of Coins
typedef unsigned long EID_t; // ID for peers
typedef float Ticks; // Time Unit, Ticks because time_t wouldn't be allowed
typedef double Size; // Size of blks/txns in units of 1 kbits

#ifndef __UTILS_CPP__
    extern ID_t NUM_PEERS;
#endif

double _exp(double mean = 1.0);

template<typename T>
T _unif_real(T l, T h)
{
    std::mt19937 rng((std::random_device())());
    std::uniform_real_distribution<T> distr(l, h);
    return distr(rng);
}

template<typename T>
T _unif_int(T l, T h)
{
    std::mt19937 rng((std::random_device())());
    std::uniform_int_distribution<T> distr(l, h);
    return distr(rng);
}

void logerr(std::string str);

void log(std::string str);

//Define Functions here to return a unique blockid and transactionid, just keep a counter / static int inside the function

#endif