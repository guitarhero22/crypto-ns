#include "block.h"
#include "utils.h"
#include <vector>
#include <queue>

Size Txn::DEFAULT_SIZE = 8;

TID_t Txn::NUM_TXNS = 0;
BID_t Blk::NUM_BLKS = 0;
Size Blk::MAX_SIZE = 8000;
TID_t Blk::MAX_TXNS = 998; // Max number of Transactions that can fit in a block
Blk *Blk::genesis = new Blk(-1, -1, 0);

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
    //will return true, if this block has previously been received
    
    if (blks.find(bid) != blks.end()) 
        //block present and is not an orphan
        return true;

    //block not present in tree if control reaches here

    if (parent < 0) // don't need to check if this is orphan
        //block not present
        return false;

    //if control reaches this point, we will check if it is presnet in data structure `orphans`
    auto par = orphans.find(parent); 

    if (par != orphans.end())
    {                                        //Parent present
        auto orphan = par->second.find(bid); // Find the block in this parent's orphans
        return orphan == par->second.end(); //if present return true
    }

    //if control reaches this point, the block has never been seen before
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
    if (chain == NULL) // recursion
        logerr("Tree:collectValidTxns chain is NULL");

    prep.clear();

    // log("Tree::collectValidTxns Updating Balances");
    if (!getBalances(chain)) // Update Balances to validate Transactions
        logerr("Peer::validTxns Error occured near getBalances");

    for (auto &txn : txns)
    {

        if (prep.size() >= Blk::MAX_SIZE)
            break; // To ensure that block size is < 8000 kb

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
    if (!getBalances(chain)) // Update balances based on longest chain
        logerr("Peer::validTxns Error occured near getBalances");

    for (auto &txn : txns)
    {
        if (!txn.second->coinbase)
        {
            if (balances[txn.second->idx] < txn.second->amt)
            {
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
    // if parent is not present in tree
    if (!findBlk(blk->parent))
        return ORPHAN;

    // If block has already been added to tree
    if (findBlk(blk->ID))
        return PRESENT;

    // Validate Transactions
    Node *parent = blks[blk->parent];
    if (!validTxns(blk->txns, parent))
        // some transaction was invalid
        return INVALID;

    // block is valid
    return VALID;
}

BID_t Tree::startMining(ID_t creator, Ticks startTime)
{

    //init
    TxnForNxtBlk.clear();

    //Collect Valid Transactions from Transaction Pool
    collectValidTxns(txnPool, longest, TxnForNxtBlk);

    //Create a coinbase Transaction
    Txn *coinbase = Txn::new_txn(creator, creator, COINBASE, startTime, true);

    //Add coinbase transaction to Txns for next block
    TxnForNxtBlk[coinbase->ID] = coinbase;

    //sanity check
    if (longest == NULL)
        logerr("Tree:startMining longest is NULL");

    //return the block ID on which to mine
    return longest->blk->ID;
}

Blk *Tree::mineBlk(ID_t creator, BID_t parent, Ticks creationTime)
{
    // Create a new block
    auto blk = Blk::new_blk(creator, TxnForNxtBlk, parent, creationTime);

    // Clear datastructur foor next transaction
    TxnForNxtBlk.clear();

    // return block
    return blk;
}

void Tree::collectTxnInChain(std::map<TID_t, Txn *> &txns, Node *n)
{
    //base case
    if (n == NULL)
    {
        return;
    }

    //Recursive Call
    collectTxnInChain(txns, n->parent);

    //Insert into map if not coinbase transaction
    for (auto &txn : n->blk->txns)
    {
        if (!txn.second->coinbase)
            txns[txn.first] = txn.second;
    }

    //return
    return;
}

void Tree::collectTxnInChain(std::unordered_set<TID_t> &txns, Node *n)
{
    //Base case
    if (n == NULL)
    {
        return;
    }

    //Recursive Call
    collectTxnInChain(txns, n->parent);

    //insert transaction in set if not coinbase 
    for (auto &txn : n->blk->txns)
    {
        txns.insert(txn.first);
    }

    //return
    return;
}

BID_t Tree::blocksInChainById(ID_t creator){

    //sanity check
    if(longest == NULL) return 0;

    BID_t ret = 0; //number of blocks in chain for id creator

    Node* node = longest; //current node pointer

    while(node != NULL){ //recursion

        if(node -> blk -> ID == 0) break; //base case

        if(node -> blk -> creator == creator) ret++; //add if the creator of block is `creator`

        node = node -> parent; // recursion
    }

    //return 
    return ret;
}

int Tree::addBlk(Blk *blk, Ticks arrival)
{

    //sanity check
    if (blk == NULL)
        logerr("Tree::addBlk blk is NULL");

    //If block has been previouslt received, don't send to peers
    if (findBlk(blk->ID))
        return DONT_SEND;

    //If the block is present in `orphans` don't send, because it has been seen before
    if (findBlk(blk->ID, blk->parent))
        return DONT_SEND;

    //Check validity of the block
    int validity = validateBlk(blk);

    // If invalid, don't send to peers
    if (validity == INVALID)
    {
        return DONT_SEND;
    }

    // If Block's parent is not present
    if (validity == ORPHAN)
    {
        // make updates
        orphansRcvd ++;

        // add to orphans
        orphans[blk->parent][blk->ID] = new Node(blk, NULL, 0, arrival);

        // SEND to peers
        return SEND;
    }

    // already present, don't send
    if (validity == PRESENT)
        return DONT_SEND;

    if (validity == VALID)
    {
        // find parent block
        Node *p = blks[blk->parent];

        // sanity check
        if (p == NULL)
            logerr("Tree::addBlk parent is Null after validation");

        // Create new node
        Node *node = new Node(blk, p, p->chainLength + 1, arrival);
        blks[blk->ID] = node;

        if (blk->creator == creator)
            //Update the count of blocks made by me
            blksByMe++;

        // sanity check
        if (longest == NULL)
            logerr("Tree::addBlk longest should not be NULL");

        //to check if a new longer chain is detected
        Node *longer = longest;
        if (node->chainLength > longer->chainLength)
            longer = node;

        //to add orphaned nodes
        std::queue<BID_t> Q;
        Q.push(blk->ID);

        /**
         * Add orphaned nodes at each iteration if parent is present
         * Check for orphaned children of these nodes in further iterations
         * 
         * Repeat untill no parent left to check for orphaned nodes
         */
        while (!Q.empty()) 
        {
            BID_t was_lost = Q.front();
            Q.pop();

            if (orphans.find(was_lost) == orphans.end())
                //no orphan for this parent
                continue;

            // if control reaches this point, the parent has orpaned nodes
            auto parent_node = blks[was_lost];

            //add all orphans of this parent to tree now
            for (auto &child : orphans[was_lost])
            {

                //sanity check
                if (findBlk(child.first) == PRESENT)
                {
                    log("Tree::addBlk Something Unusual, orphan already present");
                    continue;
                }

                //recursion
                Q.push(child.first);

                child.second->chainLength = parent_node->chainLength + 1;
                child.second->parent = parent_node;

                blks[child.first] = child.second;

                //check if a longer chaing is seen
                if (child.second->chainLength > longer->chainLength)
                {
                    longer = child.second;
                }
                else if (child.second->chainLength == longer->chainLength)
                {
                    //if length is same, check arrival time
                    if (longer->arrival > child.second->arrival)
                        longer = child.second;
                }
            }

            orphans.erase(was_lost); // erase because, no longer orphan
        }

        //check if a new longer chaing was seem
        if (longer != longest)
        {

            //update transaction pool 
            collectTxnInChain(txnPool, longest);

            inBlkChain.clear();

            longest = longer;
            collectTxnInChain(inBlkChain, longest);

            for (auto &tid : inBlkChain)
            {
                txnPool.erase(tid);
            }

            //tell the peer to start mining a new block
            return NEW_LONGEST_CHAIN;
        }

        // send to peers
        return SEND;
    }

    //don't know what to do at this point
    return DONT_KNOW;
}

int Tree::_2dot(std::ofstream &file, ID_t creator)
{
    if (!file.is_open())
        return -1;

    file << "graph {\n";

    file << "label=\"Tree of Blocks\"\n";

    for (auto blk : orphans)
    {
        for (auto orphan : blk.second)
        {
            file << orphan.first << "\n";
        }
    }

    for (auto blk : blks)
    {
        file << blk.first << " [label=\""
             << blk.second->blk->ID
             << "\\nsz:" << blk.second->blk->size
             << "\\na:" << ticks2str(blk.second->arrival)
             << "\\nc:" << blk.second->blk->creator
             << "\"";

        if (blk.second->parent != NULL && blk.second->arrival < blk.second->parent->arrival && blk.second->blk->creator == creator)
            file << ", color=purple";
        else if (blk.second->parent != NULL && blk.second->arrival < blk.second->parent->arrival)
            file << " ,color=red";
        else if (blk.second->blk->creator == creator)
            file << " ,color=green";

        file << ", shape=square]\n";
    }

    for (auto blk : blks)
    {
        if (blk.first == 0)
            continue;
        file << blk.first << " -- " << blk.second->blk->parent << "\n";
    }

    file << "}";

    return 0;
}

int Tree::_2dump(std::ofstream &file, ID_t creator)
{
    if (!file.is_open())
        return -1;

    std::multiset<Node*, compare_node_by_arrival> st;


    file << "BlkID,BlkNum,arrivalTime(min:secs),parentID\n";
    for (auto blk : blks)
    {
        st.insert(blk.second);
    }

    for (auto blk : orphans)
    {
        for (auto orphan : blk.second)
        {
            st.insert(orphan.second);
        }
    }

    for(auto node : st){
        file << node -> blk -> ID 
            << "," << node -> chainLength
            << "," << ticks2str(node -> arrival)
            << "," << node -> blk -> parent
            << "\n";
    }

    return 0;
}
