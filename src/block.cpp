#include "block.h"
#include "utils.h"
#include <vector>

Size Txn::DEFAULT_SIZE = 8;
TID_t Txn::NUM_TXNS = 0;
BID_t Blk::NUM_BLKS = 0;
Size Blk::MAX_SIZE = 8000;
Blk *Blk::genesis = new Blk(-1, 0, 0);

BID_t Tree::balancesCalcOn = 0;
std::vector<coin_t> Tree::balances;

bool Tree::findTxn(TID_t tid)
{
    return (
        txnPool.find(tid) != txnPool.end()          // check if in TxnPoo
        || inBlkChain.find(tid) != inBlkChain.end() // check if in block chain
    );
}

bool Tree::addTxn(Txn *txn)
{
    if (findTxn(txn->ID))
    {
        return false;
    }
    txnPool[txn->ID] = txn;
    return true;
}

bool Tree::findBlk(BID_t bid, BID_t parent)
{
    if (blks.find(bid) != blks.end())
        return true;
    if (parent < 0)
        return false;

    auto par = orphans.find(parent); // Find Parent who has orphaned blocks
    if (par != orphans.end())
    {                                        //Parent present
        auto orphan = par->second.find(bid); // Find the block in this parent's orphans
        return orphan == par->second.end();
    }

    return false;
}

bool Tree::getBalances(Node *n)
{
    if (n == NULL) // Base Case
    {
        if (n->blk->ID != 0)
            return false;
        balances.assign(NUM_PEERS, 0);
        return true;
    }

    if (n->blk->ID == balancesCalcOn) // Base Case
        return true;

    if (!getBalances(n->parent)) //Recurse
        return false;

    // Update balances from transactions in this Node
    auto &txns = n->blk->txns;
    for (auto &txn : txns)
    {
        balances[txn.second->idy] += txn.second->amt;
        if (!txn.second->coinbase)
        {
            balances[txn.second->idx] -= txn.second->amt;
            if (balances[txn.second->idx] < 0)
                logerr("Tree::getBalances balance of " + std::to_string(txn.second->idx) + " went < 0");
        }
    }

    // Update last Node on which balances are calculated
    balancesCalcOn = n->blk->ID;

    return true;
}

void Tree::collectValidTxns(std::map<TID_t, Txn *> &txns, Node *chain, std::map<TID_t, Txn *> &prep)
{

    prep.clear();
    if (!getBalances(chain)) // Update Balances to validate Transactions
        logerr("Peer::validTxns Error occured near getBalances");

    for (auto &txn : txns)
    {
        if (txn.second->coinbase) // Check Sanity
            logerr("Tree::validTxns with prep, coinbase should not be here");

        if (balances[txn.second->idx] < txn.second->amt) // Detech Invalid Txn
            continue;

        // Update Balances from this Transaction
        balances[txn.second->idx] -= txn.second->amt;
        balances[txn.second->idy] += txn.second->amt;

        //Add if valid
        prep[txn.first] = txn.second;
    }

    balancesCalcOn = 0; // Because we are messing with balances vector
    return;
}

bool Tree::validTxns(std::map<TID_t, Txn *> &txns, Node *chain)
{
    if (!getBalances(chain)) // Sanity Check
        logerr("Peer::validTxns Error occured near getBalances");

    for (auto &txn : txns)
    {
        if (!txn.second->coinbase)
        {
            if (balances[txn.second->idx] < txn.second->amt)
                return false;
            balances[txn.second->idx] -= txn.second->amt;
        }
        balances[txn.second->idy] += txn.second->amt;
    }

    balancesCalcOn = 0; //Because we are messing with balances vector
    return true;
}

int Tree::validateBlk(Blk *blk)
{
    if (!findBlk(blk->parent))
        return ORPHAN;

    if (findBlk(blk->ID))
        return PRESENT;

    Node *parent = blks[blk->parent];
    if (validTxns(blk->txns, parent))
        return INVALID;

    return VALID;
}

BID_t Tree::startMining(ID_t creator, Ticks startTime)
{

    TxnForNxtBlk.clear();

    collectValidTxns(txnPool, longest, TxnForNxtBlk);
    Txn *coinbase = Txn::new_txn(creator, creator, COINBASE, startTime, true);

    log("coinbase_id " + std::to_string(coinbase->ID));
    TxnForNxtBlk[coinbase->ID] = coinbase;

    return longest->blk->ID;
}

Blk *Tree::mineBlk(ID_t creator, BID_t parent, Ticks creationTime)
{

    auto blk = Blk::new_blk(creator, TxnForNxtBlk, parent, creationTime);
    TxnForNxtBlk.clear();

    return blk;
}

void Tree::collectTxnInChain(std::map<TID_t, Txn *> &txns, Node *n)
{
    if (n == NULL)
    {
        if (n->blk->ID != 0)
            logerr("Tree::collectTxnInChain root ID should not be 0");
        return;
    }

    collectTxnInChain(txns, n->parent);

    for (auto &txn : n->blk->txns)
    {
        txns[txn.first] = txn.second;
    }

    return;
}

void Tree::collectTxnInChain(std::unordered_set<TID_t> &txns, Node *n)
{
    if (n == NULL)
    {
        if (n->blk->ID != 0)
            logerr("Tree::collectTxnInChain root ID should not be 0");
        return;
    }

    collectTxnInChain(txns, n->parent);

    for (auto &txn : n->blk->txns)
    {
        txns.insert(txn.first);
    }
}

int Tree::addBlk(Blk *blk, Ticks arrival)
{

    if (findBlk(blk->ID))
        return DONT_SEND;

    if (findBlk(blk->ID, blk->parent))
        return DONT_SEND;

    int validity = validateBlk(blk);

    if (validity == INVALID)
        return DONT_SEND;

    if (validity == ORPHAN)
    {
        orphans[blk->parent][blk->ID] = blk;
        return SEND;
    }

    if (validity == PRESENT)
        return DONT_SEND;

    if (validity == VALID)
    {
        Node *p = blks[blk->parent];

        if (p == NULL)
            logerr("Tree::addBlk parent is Null after validation");

        Node *node = new Node(blk, p, p->chainLength + 1, arrival);

        while (0)
        {

            //! Add logic to add orphaned blocks
        }

        if (longest == NULL)
            logerr("Tree::addBlk longest should not be NULL");

        if (longest == p)
        {
            for (auto &txn : blk->txns)
            {
                if (!txn.second->coinbase)
                    txnPool.erase(txn.first);
            }

            for (auto &txn : blk->txns)
            {
                if (!txn.second->coinbase)
                    inBlkChain.insert(txn.first);
            }

            longest = node;

            return NEW_LONGEST_CHAIN;
        }

        if (node->chainLength > longest->chainLength)
        {

            collectTxnInChain(txnPool, longest);

            inBlkChain.clear();

            longest = node;
            collectTxnInChain(inBlkChain, longest);

            for (auto &tid : inBlkChain)
            {
                txnPool.erase(tid);
            }

            return NEW_LONGEST_CHAIN;
        }

        return SEND;
    }

    return DONT_KNOW;
}