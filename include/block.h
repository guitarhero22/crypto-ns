#ifndef __BLOCK_H__
#define __BLOCK_H__

#include "utils.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>

#define PRESENT 3
#define ORPHAN 2
#define INVALID 1
#define VALID 0

#define NEW_LONGEST_CHAIN 0
#define SEND 1
#define DONT_SEND 2
#define DONT_KNOW 3

#define BLOCK 0
#define TXN 1

#define COINBASE 50

class Msg
{
public:
    Msg(){};
    Msg(Size sz, int t) : size(sz), type(t) {}
    Size size;
    int type;
};

class Txn : public Msg
{
public:
    static Size DEFAULT_SIZE;
    static TID_t NUM_TXNS;
    static TID_t get_next_txn()
    {
        return NUM_TXNS++;
    }

    static Txn *new_txn(ID_t src, ID_t tgt, coin_t c, Ticks t, bool coinbase = false)
    {
        auto txn = new Txn(src, tgt, c, t);
        txn->coinbase = coinbase;
        return txn;
    };

    // Txn(): Msg(DEFAULT_SIZE, TXN){};
    Txn(TID_t tid, ID_t src, ID_t tgt, coin_t c, Ticks t) : ID(tid), idx(src), idy(tgt), amt(c), timestamp(t), Msg(DEFAULT_SIZE, TXN)
    {
        if (tgt < 0 && tgt < NUM_PEERS)
            logerr("Txn::Txn Target should not be > 0 and < NUM_PEERS");
    }

    Txn(ID_t src, ID_t tgt, coin_t c, Ticks t) : ID(get_next_txn()), idx(src), idy(tgt), amt(c), timestamp(t), Msg(DEFAULT_SIZE, TXN)
    {
        if (tgt < 0 && tgt < NUM_PEERS)
            logerr("Txn::Txn Target should not be > 0 and < NUM_PEERS");
    };

    TID_t ID;
    ID_t idx;
    ID_t idy;
    coin_t amt;
    Ticks timestamp;
    bool coinbase = false;
    // ~Txn(){log("Txn::destructor");}
};

class compare_txn
{
public:
    bool operator()(const Txn &a, const Txn &b) { return a.timestamp < b.timestamp; }
};

class Blk : public Msg
{
public:
    static BID_t NUM_BLKS;
    static Size MAX_SIZE;
    static Blk *genesis;

    static BID_t get_next_blk()
    {
        return NUM_BLKS++;
    }

    static Blk *new_blk(ID_t c, std::map<TID_t, Txn *> &txns, BID_t parent, Ticks creationTime)
    {
        return new Blk(c, txns, parent, creationTime);
    };

    Blk() : Msg(0, BLOCK) {}
    Blk(ID_t c, std::map<TID_t, Txn *> &_txns, BID_t _parent, Ticks creationTime) : Msg(0, BLOCK), ID(get_next_blk()), timestamp(creationTime), txns(_txns), parent(_parent), creator(c)
    {
        // size of self and parent ID and timestamp and the size of all transactions
        size += (16 * sizeof(ID) + 8 * sizeof(timestamp)) / 1000.0;
        size += Txn::DEFAULT_SIZE * txns.size();
    };

    Blk(ID_t c, BID_t _parent, Ticks creationTime) : Msg(0, BLOCK), ID(get_next_blk()), timestamp(creationTime), parent(_parent), creator(c)
    {
        // size of self and parent ID and timestamp and the size of all transactions
        size += (16 * sizeof(ID) + 8 * sizeof(timestamp)) / 1000.0;
        size += Txn::DEFAULT_SIZE * txns.size();
    };

    BID_t ID;                    // ID
    BID_t parent;                // Parent Block ID
    Ticks timestamp;             // Time when the block was created
    ID_t creator;                // The peer who generated this block
    std::map<TID_t, Txn *> txns; // Transactions added to the block
    Txn *coinbase = NULL;

    // ~Blk(){log("Blk::destructor");}
};

//Data structure used to store blocks in BlkTree by peers
class Node
{
public:
    Node() {}
    Node(Blk *b, Node *p, BID_t l, Ticks a) : blk(b), parent(p), chainLength(l), arrival(a) {}
    Blk *blk = NULL;
    Node *parent = NULL;
    BID_t chainLength = 0;
    Ticks arrival;
    // ~Node(){log("Node::destructor");}
};

//Data structure used by peer handle Txns and Blocks
class Tree
{
public:
    static std::vector<coin_t> balances;
    static BID_t balancesCalcOn;
    static void INIT(void) { balances.resize(NUM_PEERS); }

    Tree()
    {
        auto node = new Node(Blk::genesis, NULL, 0, 0);
        blks[Blk::genesis->ID] = node;
        genesis = node;
        longest = node;
    }

    bool findTxn(TID_t);
    bool findBlk(BID_t);
    bool addTxn(Txn *);                   //! implement
    int addBlk(Blk *blk, Ticks arrival); //! implement
    int validateBlk(Blk *blk);            //! implement
    bool getBalances(Node *n);
    // std::set<TID_t> validTxns(std::unordered_map<TID_t, Txn *> &, BID_t); //! implement
    bool validTxns(std::map<TID_t, Txn *> &, Node *);                           //! implement
    void collectValidTxns(std::map<TID_t, Txn *> &, Node *, std::map<TID_t, Txn *> &); //! implement
    void collectTxnInChain(std::map<TID_t, Txn*> &, Node*);
    void collectTxnInChain(std::unordered_set<TID_t> &, Node*);

    Blk *mineBlk(ID_t, BID_t, Ticks);
    BID_t startMining(ID_t, Ticks);

    BID_t getLastId()
    {
        if (longest != NULL)
            return (*longest).blk->ID;
        else
            return -1;
    }; //

    Node *genesis = NULL;
    Node *longest = NULL;
    std::map<TID_t, Txn *> txnPool;
    std::map<TID_t, Txn *> TxnForNxtBlk; // Will hold transactions for the next Block
    std::unordered_set<TID_t> inBlkChain;
    std::unordered_map<BID_t, Node *> blks;
    std::unordered_map<BID_t, Blk *> orphans; // parent -> Blk

    // ~Tree(){log("Tree::destructor");}
};

#endif