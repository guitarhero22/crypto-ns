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

    if (n == NULL || n->blk->ID == 0) // Base Case
    {
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

    if (chain == NULL)
        logerr("Tree:collectValidTxns chain is NULL");

    prep.clear();

    // log("Tree::collectValidTxns Updating Balances");
    if (!getBalances(chain)) // Update Balances to validate Transactions
        logerr("Peer::validTxns Error occured near getBalances");

    for (auto balance : balances)

    // if(txns.empty()) log("Txns::collectValidTxns Transactions are empty");
    for (auto &txn : txns)
    {
        if (txn.second->coinbase) // Check Sanity
            logerr("Tree::collectValidTxns with prep, coinbase should not be here");

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
            {
                // log("Tree::validTxns txn invalid: tid: " + tos(txn.first) +
                //     " to: " + tos(txn.second -> idy)
                //     + " from: " + tos(txn.second -> idx)
                //     + " amt: " + tos(txn.second -> amt)
                //     + " bal: " + tos(balances[txn.second -> idx]));
                return false;
            }
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
    if (!validTxns(blk->txns, parent))
        return INVALID;

    return VALID;
}

BID_t Tree::startMining(ID_t creator, Ticks startTime)
{

    // log("Tree::startMining starting mining for " + std::to_string(creator));
    TxnForNxtBlk.clear();

    // log("Tree::startMining collecting Txns for " + std::to_string(creator));
    collectValidTxns(txnPool, longest, TxnForNxtBlk);
    // log("Tree::startMining Transactions Collected " + std::to_string(creator));

    Txn *coinbase = Txn::new_txn(creator, creator, COINBASE, startTime, true);
    // log("coinbase_id " + std::to_string(coinbase->ID));

    TxnForNxtBlk[coinbase->ID] = coinbase;

    if (longest == NULL)
        logerr("Tree:startMining longest is NULL");
    return longest->blk->ID;
}

Blk *Tree::mineBlk(ID_t creator, BID_t parent, Ticks creationTime)
{
    auto blk = Blk::new_blk(creator, TxnForNxtBlk, parent, creationTime);
    // log("Block " + tos(blk->ID) + " Mined by " + tos(creator) + " on " + tos(parent) + " not: " + tos(TxnForNxtBlk.size()));
    TxnForNxtBlk.clear();

    return blk;
}

void Tree::collectTxnInChain(std::map<TID_t, Txn *> &txns, Node *n)
{
    if (n == NULL)
    {
        // if (n->blk->ID != 0)
        // logerr("Tree::collectTxnInChain root ID should not be 0");
        return;
    }

    collectTxnInChain(txns, n->parent);

    for (auto &txn : n->blk->txns)
    {
        if (!txn.second->coinbase)
            txns[txn.first] = txn.second;
    }

    return;
}

void Tree::collectTxnInChain(std::unordered_set<TID_t> &txns, Node *n)
{
    if (n == NULL)
    {
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

    if (blk == NULL)
        logerr("Tree::addBlk blk is NULL");

    if (findBlk(blk->ID))
        return DONT_SEND;

    if (findBlk(blk->ID, blk->parent))
        return DONT_SEND;

    int validity = validateBlk(blk);

    if (validity == INVALID)
    {
        // log("Block " + tos(blk->ID) + " Invalid");
        return DONT_SEND;
    }

    if (validity == ORPHAN)
    {
        // log("Blk " + tos(blk->ID) + " is orphan");
        orphans[blk->parent][blk->ID] = blk;
        return SEND;
    }

    if (validity == PRESENT)
        return DONT_SEND;

    if (validity == VALID)
    {
        // log("Blk " + tos(blk->ID) + " is valid, adding to tree");
        Node *p = blks[blk->parent];

        if (p == NULL)
            logerr("Tree::addBlk parent is Null after validation");

        Node *node = new Node(blk, p, p->chainLength + 1, arrival);

        // while (0)
        // {

        //     //! Add logic to add orphaned blocks
        // }

        if (longest == NULL)
            logerr("Tree::addBlk longest should not be NULL");

        blks[blk->ID] = node;
        // log("Blk " + tos(blk->ID) + " added, the parent is " + tos(blk->parent));

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

int Tree::_2dot(std::ofstream &file)
{
    if (!file.is_open())
        return -1;

    file << "graph {\n";

    for (auto blk : orphans)
    {
        for (auto orphan : blk.second)
        {
            file << orphan.first << "\n";
        }
    }

    for (auto blk : blks)
    {
        file << blk.first << " -- " << blk.second->blk->parent << "\n";
    }

    file << "}";

    return 0;
}