#include "block.h"
#include "utils.h"
#include <vector>

Size Txn::DEFAULT_SIZE = 8;
TID_t Txn::NUM_TXNS = 0;
BID_t Blk::NUM_BLKS = 0;
Size Blk::MAX_SIZE = 1000;

bool Tree::findTxn(TID_t tid){
    return (
        txnPool.find(tid) != txnPool.end()
        || inBlkChain.find(tid) != inBlkChain.end()
    );
}

bool Tree::addTxn(Txn *txn){
    if(findTxn(txn -> ID)) return true;
    txnPool[txn -> ID] = txn;
    return false;
}

bool Tree::findBlk(BID_t bid){
    return (
        blks.find(bid) != blks.end()
    );
}

bool Tree::insert(Blk *blk, bool inChain){
    if(findBlk(blk -> ID)){
        return true;
    }

    if(!findBlk(blk -> parent)) return false;
    if(inChain)
        if(getLastId() != blk -> parent) return false;

    //! Complete this function

}