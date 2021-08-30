#ifndef __PEER_H__
#define __PEER_H__

#include <random>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <set>

#include "event.h"
#include "utils.h"
#include "block.h"

extern int NUM_PEERS;
class Peer; //forward Declaration
/**
 * Messages will live in the link untill they reach the target
 * Make methods so that link can send/receive transactions to/from target/source
 */
class Link
{
public:
    Link() {}
    Link(Peer *, Peer *);
    Event *sendTxn(Txn, Ticks);
    Event *sendBlk(Blk, Ticks);
    // Event send_block(Block){};

    Peer *src;
    Peer *tgt;
    Ticks p;
    Ticks c; //link speed
    std::mt19937 rng;
    std::exponential_distribution<Ticks> d;
};

/**
 * ! Mention in report why exponential distro is used for Txns
 * ! For a loop-less transaction forwarding, remember all the transactions that have been sent out, 
 * ! dont send them out again and send them out only to the peers who did not send you that transaction
 */
class Peer
{
public:
    Peer(){};
    Peer(ID_t, Ticks, Ticks, bool);
    bool is_slow() { return slow; }

    //Event Callbacks
    std::vector<Event *> rcvTxn(Peer *, Txn, Ticks); //Callback for receiving a transaction
    std::vector<Event *> genTxn(Ticks);
    std::vector<Event *> rcvBlk(Peer *, Blk, Ticks);
    std::vector<Event *> genBlk(Ticks, Ticks, BID_t);

    //for randomness
    ID_t ID;
    std::mt19937 rng;
    std::exponential_distribution<Ticks> nextTxnTime; //For sampling time intervals between transactions
    std::exponential_distribution<Ticks> nextBlkTime; //For sampling time intervals between transactions

    //For structure
    bool slow;
    Ticks computePower;  // Compute Power, for deciding when a block will be mined
    Ticks txnGenFreq; //Transaction Generation Frequency
    Ticks miningSessionStart = 0;
    Tree tree;
    std::unordered_map<Peer *, Link> links;
    std::unordered_map<TID_t, Txn> forNxtBlk; // Will hold transactions for the next Block
};

// Events related to Peers and Links
class PeerRcvTxn : public Event
{
public:
    PeerRcvTxn(Peer *_from, Peer *_to, Txn _txn, Ticks t) : Event(t), txn(_txn), from(_from), to(_to){};
    std::vector<Event *> callback(Ticks);
    Peer *from, *to;
    Txn txn;
};

class PeerGenTxn : public Event
{
public:
    PeerGenTxn(Peer *p, Ticks t) : peer(p), Event(t){};
    std::vector<Event *> callback(Ticks);
    Peer *peer;
};

class PeerGenBlk : public Event
{
public:
    PeerGenBlk(Peer *p, Ticks _start, Ticks _timestamp, BID_t _parent) : peer(p), startTime(_start), Event(_timestamp), parent(_parent){};
    std::vector<Event *> callback(Ticks);
    Ticks startTime;
    Peer *peer;
    BID_t parent;
};

class PeerRcvBlk : public Event
{
public:
    PeerRcvBlk(Peer *_from, Peer *_to, Blk _blk, Ticks t) : Event(t), blk(_blk), from(_from), to(_to){};
    std::vector<Event *> callback(Ticks);
    Peer *from, *to;
    Blk blk;
};

void ConnectGraphByRandomWalk(std::vector<Peer> &);

#endif