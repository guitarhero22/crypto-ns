#ifndef __BLOCK_H__
#define __BLOCK_H__

#include "utils.h"
// Keep a Union for transactions and then define util functions to convert that union into strings and vice versa
// Decide how long the transaction ID will be ??????

// Define the Block Class here and all other functionality to maintain the Block Tree here
union txn_t {
    TID_t tid; //Transaction ID
    ID_t idx; // IDx
    ID_t idy; // IDy
    coin_t amt; // Amount
};

#endif