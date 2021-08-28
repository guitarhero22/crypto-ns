#ifndef __BLOCK_H__
#define __BLOCK_H__

#include "utils.h"
#include <vector> 

// Define the Block Class here
class Txn
{
public:

    static TID_t NUM_TXNS;
    static TID_t get_next_txn(){
        return NUM_TXNS++;
    }

    Txn(){};
    Txn(TID_t tid, ID_t src, ID_t tgt, coin_t c, Ticks t) : ID(tid), idx(src), idy(tgt), amt(c), timestamp(t), size(8) {}
    Txn(ID_t src, ID_t tgt, coin_t c, Ticks t): ID(get_next_txn()), idx(src), idy(tgt), amt(c), timestamp(t), size(8) {};

    TID_t ID;
    ID_t idx;
    ID_t idy;
    coin_t amt;
    Ticks timestamp;
    Size size = 8;
};

class Block
{
public:
    Block() {}
};

#endif