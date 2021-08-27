#ifndef __UTILS_H__
#define __UTILS_H__

#include<random>
#include<chrono>

typedef unsigned long ID_t; // ID for peers
typedef unsigned long BID_t; // ID for Blocks
typedef unsigned long TID_t; // ID for Txns
typedef double coin_t; // For Number of Coins
// typedef double time_t; //time type

double _exp(double mean = 1.0);
//Define Functions here to return a unique blockid and transactionid, just keep a counter / static int inside the function

#endif