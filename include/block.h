#ifndef __BLOCK_H__
#define __BLOCK_H__

#include "utils.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <map>

// Define the Block Class here

#define BLOCK 0
#define TXN 1

class Msg{
    public:
        Msg(){};
        Msg(Size sz, int t): size(sz), type(t){}
        Size size;
        int type;
};

class Txn: public Msg
{
public:
    static Size DEFAULT_SIZE;
    static TID_t NUM_TXNS;
    static TID_t get_next_txn()
    {
        return NUM_TXNS++;
    }

    static Txn* new_txn(ID_t src, ID_t tgt, coin_t c, Ticks t){
        return new Txn(src, tgt, c, t);
    };

    Txn(): Msg(DEFAULT_SIZE, TXN){};
    Txn(TID_t tid, ID_t src, ID_t tgt, coin_t c, Ticks t) : ID(tid), idx(src), idy(tgt), amt(c), timestamp(t), Msg(DEFAULT_SIZE, TXN) {}
    Txn(ID_t src, ID_t tgt, coin_t c, Ticks t) : ID(get_next_txn()), idx(src), idy(tgt), amt(c), timestamp(t), Msg(DEFAULT_SIZE, TXN) {};

    TID_t ID;
    ID_t idx;
    ID_t idy;
    coin_t amt;
    Ticks timestamp;
};

class compare_txn
{
public:
    bool operator()(const Txn &a, const Txn &b) { return a.timestamp < b.timestamp; }
};

class Blk: public Msg
{
public:
    static BID_t NUM_BLKS;
    static Size MAX_SIZE;
    static BID_t get_next_blk()
    {
        return NUM_BLKS++;
    }
    static Blk* new_blk(std::unordered_map<TID_t, Txn*> &txns, BID_t parent, Ticks creationTime){
        return new Blk(txns, parent, creationTime);
    };

    Blk(): Msg(0, BLOCK) {}
    Blk(std::unordered_map<TID_t, Txn*> &_txns, BID_t _parent, Ticks creationTime) : Msg(0, BLOCK), ID(get_next_blk()), timestamp(creationTime), txns(_txns), parent(_parent)
    {
        // size of self and parent ID and timestamp and the size of all transactions
        (size += (16 * sizeof(ID) + 8 * sizeof(timestamp)) / 1000) += Txn::DEFAULT_SIZE * txns.size();
    };

    BID_t ID;                            // ID
    BID_t parent;                        // Parent Block ID
    Ticks timestamp;                     // Time when the block was created
    std::unordered_map<TID_t, Txn*> txns; // Transactions added to the block
};

//Data structure used to store blocks in BlkTree by peers
class Node
{
public:
    Node() {}
    Blk *blk;
    Node *parent = NULL;
    BID_t chainLength = 0;
};

//Data structure used by peer handle Txns and Blocks
class Tree
{
public:
    Tree() {}
    bool findTxn(TID_t);
    bool findBlk(BID_t);
    bool addTxn(Txn*);
    bool insert(Blk *blk, bool inChain); // insert block in chain
    BID_t getLastId(){
        if(longest != NULL) return (*longest).blk -> ID; 
        else return -1;
    }; //
     
    Node *genesis;
    Node *longest;
    std::map<TID_t, Txn*> txnPool;
    std::unordered_set<TID_t> inBlkChain;
    std::unordered_map<BID_t, Node*> blks;
};

#endif