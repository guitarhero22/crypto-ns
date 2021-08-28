#ifndef __PEER_H__
#define __PEER_H__

#include <random>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "event.h"
#include "utils.h"
#include "block.h"

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

    //for randomness
    ID_t ID;
    std::mt19937 rng;
    std::exponential_distribution<Ticks> nextTxnTime; //For sampling time intervals between transactions

    //For structure
    bool slow;
    std::unordered_map<Peer *, Link> links;
    std::unordered_map<TID_t, Txn> txnSent;
    std::unordered_map<TID_t, Txn> txnPool;

protected:
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

void ConnectGraphByRandomWalk(std::vector<Peer> &);

#endif