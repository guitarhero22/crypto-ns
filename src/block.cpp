#include "block.h"
#include "utils.h"
#include <vector>

Size Txn::DEFAULT_SIZE = 8;
TID_t Txn::NUM_TXNS = 0;
BID_t Blk::NUM_BLKS = 0;
Size Blk::MAX_SIZE = 1000;

bool Tree::findTxn(TID_t tid){
    return (
        txnPool.find(tid) != txnPool.end() // check if in TxnPoo
        || inBlkChain.find(tid) != inBlkChain.end() // check if in block chain
        || invalidTxns.find(tid) != invalidTxns.end() // check if in invalid Txns
    );
}

// Return True if Txn is inserted, false if it is already present
bool Tree::addTxn(Txn *txn){
    if(findTxn(txn -> ID)) {
        return false;
    }
    txnPool[txn -> ID] = txn;
    return true;
}

bool Tree::findBlk(BID_t bid){
    return (
        blks.find(bid) != blks.end()
    );
}

//Return True if block is inserted
/**
 * What should this function do? How should it behave?
 * Return true if inserted
 * False if could not insert or already present
 */
bool Tree::addBlk(Blk *blk){

    if(findBlk(blk -> ID)) return false;

    // !Check if Block is Valid -- basically check that according to the parent chain all the peers paying someone have positive balance, if invalid return false, this sounds dicey, rethink

    if(blks.find(blk -> parent) == blks.end()){
        //This is an orphaned block, make a special data structure and add it

        return true; //still returning true because it should be braodcasted?
    }

    // !Insert the block in the tree
    blks[blk -> ID] = new Node(blk, NULL, 0, 0);
    return true;
}