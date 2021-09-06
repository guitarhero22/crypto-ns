#ifndef __BLOCK_H__
#define __BLOCK_H__

#include "utils.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>
#include <fstream>

#define PRESENT 3
#define ORPHAN 2
#define INVALID 1
#define VALID 0

#define NEW_LONGEST_CHAIN 4
#define SEND 5
#define DONT_SEND 6
#define DONT_KNOW 7

#define BLOCK 8
#define TXN 9

#define COINBASE 50 ///< Default COINBASE TXN AMOUNT

/**
 * A Msg that can travel through Links.
 * 
 * Simply defined to declare children `Txn` and `Blk`, so that they can be transferred through Links
 */
class Msg
{
public:
    Msg(){}; ///< Default Constructor

    /**
     * Msg Constructor
     * 
     * @param size_ Size of the message
     * @param type_ Time when the 
     */
    Msg(Size size_, int type_) : size(size_), type(type_) {}

    Size size; ///< size of the message in kilo bits
    int type;  ///< Type, one of BLOCK or TXN (0 or 1)
};

/**
 * Transaction
 */
class Txn : public Msg
{
public:
    static Size DEFAULT_SIZE; ///< Default Size, currently 8 kilo bits = 1KB
    static TID_t NUM_TXNS;    ///< To keep track of number of transactions

    /**
     * Gets the next available Transaction ID (TID_t)
     */
    static TID_t getNxtTxn()
    {
        return NUM_TXNS++;
    }

    /**
     * Constructs a new Txn
     * 
     * @param src ID_x
     * @param tgt ID_y
     * @param C Number of Bitcoins
     * @param timestamp Time of Creation
     * 
     * @returns pointer to the newly created Txn
     */
    static Txn *new_txn(ID_t src, ID_t tgt, coin_t C, Ticks timestamp, bool coinbase = false)
    {
        auto txn = new Txn(src, tgt, C, timestamp);
        txn->coinbase = coinbase;
        return txn;
    };

    // Txn(TID_t tid, ID_t src, ID_t tgt, coin_t c, Ticks t) : ID(tid), idx(src), idy(tgt), amt(c), timestamp(t), Msg(DEFAULT_SIZE, TXN)
    // {
    //     if (tgt < 0 && tgt < NUM_PEERS)
    //         logerr("Txn::Txn Target should not be > 0 and < NUM_PEERS");
    // }

    /**
     * Constructor
     * 
     * @param src ID_x
     * @param tgt ID_y
     * @param c Number of BitCoins
     * @param t timestamp
     * 
     * @returns Instance of Txn Class
     */
    Txn(ID_t src, ID_t tgt, coin_t c, Ticks t) : ID(getNxtTxn()), idx(src), idy(tgt), amt(c), timestamp(t), Msg(DEFAULT_SIZE, TXN)
    {
        if (tgt < 0 && tgt < NUM_PEERS)
            logerr("Txn::Txn Target should not be > 0 and < NUM_PEERS");
    };

    TID_t ID;              ///< Unique Transaction ID
    ID_t idx;              ///< ID of the payer
    ID_t idy;              ///< ID of the payee
    coin_t amt;            ///< Transaction Amount
    Ticks timestamp;       ///< Timestamp
    bool coinbase = false; ///< Whether this is a coinbase Txn
};

// class compare_txn
// {
// public:
//     bool operator()(const Txn &a, const Txn &b) { return a.timestamp < b.timestamp; }
// };

/**
 * Block
 */
class Blk : public Msg
{
public:
    static BID_t NUM_BLKS; ///< To keep a count of all the blocks created
    static Size MAX_SIZE;  ///< MAX SIZE of a block
    static Blk *genesis;   ///< Pointer to the genesis block

    /**
     * Gets the next available Block ID 
     * 
     * @returns next available Block ID
     */
    static BID_t getNxtBlk()
    {
        return NUM_BLKS++;
    }

    /**
     * Creates a New Block
     * 
     * Return a Pointer to the block, instead of an instance of the block, 
     * so that everyone can use the pointer to refer to the block to save
     * space during simulation
     * 
     * @returns pointer to a newly created block
     */
    static Blk *new_blk(ID_t c, std::map<TID_t, Txn *> &txns, BID_t parent, Ticks creationTime)
    {
        return new Blk(c, txns, parent, creationTime);
    };

    /**
     * Constructor
     */
    Blk() : Msg(0, BLOCK) {}

    /**
     * Constructor
     * 
     * @returns instance of Blk Class
     */
    Blk(ID_t c, std::map<TID_t, Txn *> &_txns, BID_t _parent, Ticks creationTime) : Msg(0, BLOCK), ID(getNxtBlk()), timestamp(creationTime), txns(_txns), parent(_parent), creator(c)
    {
        // size of self and parent ID and timestamp and the size of all transactions
        size += (16 * sizeof(ID) + 8 * sizeof(timestamp)) / 1000.0;
        size += Txn::DEFAULT_SIZE * txns.size();
    };

    /**
     * Constructor
     * 
     * @returns instance of Blk Class
     */
    Blk(ID_t c, BID_t _parent, Ticks creationTime) : Msg(0, BLOCK), ID(getNxtBlk()), timestamp(creationTime), parent(_parent), creator(c)
    {
        // size of self and parent ID and timestamp and the size of all transactions
        size += (16 * sizeof(ID) + 8 * sizeof(timestamp)) / 1000.0;
        size += Txn::DEFAULT_SIZE * txns.size();
    };

    BID_t ID;                    ///< ID
    BID_t parent;                ///< Parent Block ID
    Ticks timestamp;             ///< Time when the block was created
    ID_t creator;                ///< The peer who generated this block
    std::map<TID_t, Txn *> txns; ///< Transactions added to the block
    Txn *coinbase = NULL;
};

/**
 * Class Node, for maintaing the Blocks within the tree
 */
class Node
{
public:
    Node() {} ///< Constructor

    /**
     * Constructor
     */
    Node(Blk *b, Node *p, BID_t l, Ticks a) : blk(b), parent(p), chainLength(l), arrival(a) {}
    Blk *blk = NULL;       ///< The block that the Node is holding
    Node *parent = NULL;   ///< Parent Node in the Tree
    BID_t chainLength = 0; ///< Depth in the Tree
    Ticks arrival;         ///< Time when the block arrived
};

/**
 * Tree
 * 
 * For managing the Incoming Blocks and Transactions, and Mining new blocks
 */
class Tree
{
public:
    static std::vector<coin_t> balances;                   ///< Balances of all the Peers
    static BID_t balancesCalcOn;                           ///< The last Block ID on which balances were calculated
    static void INIT(void) { balances.resize(NUM_PEERS); } ///< Initilize

    Tree()
    {
        auto node = new Node(Blk::genesis, NULL, 0, 0);
        blks[Blk::genesis->ID] = node;
        genesis = node;
        longest = node;
    } ///< Constructor

    /**
     * Checks if Txn has already been received
     * 
     * @param tid Transaction ID
     */
    bool findTxn(TID_t tid);

    /**
     * 
     * Checks if Block has already been received
     * 
     * @param bid Block ID
     * @param parent to find orphan block by parent ID
     */
    bool findBlk(BID_t bid, BID_t parent = -1);

    /**
     * 
     * Adds a Transaction to the Tree
     * 
     * @param txn
     * 
     * @returns bool, suggesting whether this Txn should be sent to peers
     */
    bool addTxn(Txn *txn);

    /**
     * Add Block to the Tree
     * 
     * @param blk pointer to the block to be added
     * @param arrival time when the block arrived
     * 
     * @return INT used to find out whether to send it to peers or start mining, etc.
     */
    int addBlk(Blk *blk, Ticks arrival);

    /**
     * validates a Block
     * 
     * @param blk pointer to the block to be validated
     * @returns INT - validation status
     */
    int validateBlk(Blk *blk);

    /**
     * Update the balances vector
     * 
     * @param n the chain using which balances will be calculated
     */
    bool getBalances(Node *n);

    /**
     *  validate Transactions
     * 
     * @param txns the Transactions to be validated
     * @param n the chain on which Transactions will be validated
     */
    bool validTxns(std::map<TID_t, Txn *> &txns, Node *n);

    /**
     * collect Valid Transactions from Pool for Mining the Next Block
     * 
     * @param txns the transaction Pool
     * @param chain the chain to check for valid transactions
     * @param conatiner the container for collected transactions
     */
    void collectValidTxns(std::map<TID_t, Txn *> &txns, Node *chain, std::map<TID_t, Txn *> &container);

    /**
     * Cherry Pick Transactions from the given Chain
     * 
     * @param txns container to collec the Transactions
     * @param n the chain 
     */
    void collectTxnInChain(std::map<TID_t, Txn *> &txns, Node *n);

    /**
     * Cherry Pick Transactions from the given Chain
     * 
     * @param txns container to collec the Transactions
     * @param n the chain 
     */
    void collectTxnInChain(std::unordered_set<TID_t> &, Node *);

    /**
     * Mine a Block
     * 
     * @param id ID of the miner
     * @param bid the parent Block ID
     * @param timestamp Timestamp for the Block
     * 
     * @returns returns the newly mined Block
     */
    Blk *mineBlk(ID_t id, BID_t parent, Ticks timestamp);

    /**
     * Start Mining
     * 
     * Prepare various structures(transactions, etc.) for mining the next Block
     * @param id ID of the miner
     * @param startTime time when the mining starts
     * @returns Block ID of the last block in the longest chain
     */
    BID_t startMining(ID_t id, Ticks timestamp);

    /**
     * Prints the tree to a dot file
     * 
     * @param file the output file stream
     * @returns status
     */
    int _2dot(std::ofstream &file);

    Node *genesis = NULL;                     ///< the node containing the genesis block
    Node *longest = NULL;                     ///< pointer to the last Node in longest Block Chain
    std::map<TID_t, Txn *> txnPool;           ///< The transaction Pool, the transactions which are not in the longest chain
    std::map<TID_t, Txn *> TxnForNxtBlk;      ///< Will hold transactions for the next Block
    std::unordered_set<TID_t> inBlkChain;     ///< The transactions which are in the longest chain
    std::unordered_map<BID_t, Node *> blks;   ///< To keep track of Blocks and Nodes
    std::unordered_map<BID_t, std::unordered_map<BID_t, Blk *>> orphans; ///< To keep track of orphaned blocks
};

#endif