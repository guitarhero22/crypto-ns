/** @file utils.h
 * Some utils.
 */
#ifndef __UTILS_H__
#define __UTILS_H__

#include <random>
#include <chrono>
#include <iostream>

typedef long ID_t;           ///< ID for peers
typedef long BID_t; ///< ID for Blocks
typedef unsigned long TID_t; ///< ID for Txns
typedef unsigned int coin_t; ///< For Number of Coins
typedef unsigned long EID_t; ///< ID for peers
typedef float Ticks;         ///< Time Unit, the name Ticks because time_t wouldn't be allowed
typedef double Size;         ///< Size of blks/txns in units of 1 kbits

#ifndef __UTILS_CPP__
extern ID_t NUM_PEERS; ///< Defined in utils.h, so that everyone has access
extern std::mt19937 rng;
#endif

#define tos(a) std::to_string(a)

/**
 * Uniform Real Distr.
 * Samples a number from the specified uniform distribution
 * 
 * @param l lower limit
 * @param h upper limit
 * @tparam T REAL data types
 */
template <typename T>
T _unif_real(T l, T h)
{
    std::mt19937 rng((std::random_device())());
    std::uniform_real_distribution<T> distr(l, h);
    return distr(rng);
}

/**
 *  Uniform Int Distr. 
 * Samples a number from the specified uniform distribution
 * 
 * @param l lower limit
 * @param h upper limit
 * @tparam T INT data type
 */
template <typename T>
T _unif_int(T l, T h)
{
    std::mt19937 rng((std::random_device())());
    std::uniform_int_distribution<T> distr(l, h);
    return distr(rng);
}

/**
 * Error log and Exit
 * 
 * @param str Error String, printed on stderr
 */
void logerr(std::string str);

/**
 * Log
 * 
 * @param str Print String on stdout
 */
void log(std::string str);

/**
 * Ticks to readable time 
 * 
 * @param time Ticks
 */
std::string ticks2str(Ticks time);


#endif