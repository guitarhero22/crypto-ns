#ifndef __PEER_H__
#define __PEER_H__

#include <random>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <set>
#include <queue>
#include <utility>


#include "event.h"
#include "utils.h"
#include "block.h"

typedef std::pair<Ticks, Msg*> pTM;

class Peer; //forward Declaration
/**
 * Messages will live in the link untill they reach the target
 * Make methods so that link can send/receive transactions to/from target/source
 */
class Link : public EventGenerator
{
public:
    Link(){}
    Link(Peer *, Peer *);
    Event* send(Msg *, Ticks);
    std::vector<Event*> sent(Ticks, EID_t);

    std::priority_queue<pTM, std::vector<pTM>, std::greater<pTM>> Q;
    Peer *src;
    Peer *tgt;
    Ticks p;
    Ticks c; //link speed
    std::mt19937 rng;
    std::exponential_distribution<Ticks> d;

    // ~Link(){log("Link::destructor");}
};

/**
 * ! Mention in report why exponential distro is used for Txns
 * ! For a loop-less transaction forwarding, remember all the transactions that have been sent out, 
 * ! dont send them out again and send them out only to the peers who did not send you that transaction
 */
class Peer: public EventGenerator
{
public:
    static std::vector<Event*> INIT(std::vector<Peer> &);
    Peer(){};
    Peer(ID_t, Ticks, Ticks, bool);
    bool is_slow() { return slow; }

    //Event Callbacks
    std::vector<Event *> rcvMsg(Peer*, Msg*, Ticks);
    std::vector<Event *> rcvTxn(Peer*, Txn*, Ticks); //Callback for receiving a transaction
    std::vector<Event *> genTxn(Ticks, EID_t);
    std::vector<Event *> rcvBlk(Peer*, Blk*, Ticks);
    std::vector<Event *> genBlk(Ticks, EID_t);
    Event *start_mining_session(Ticks startTime);

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

    // Mining
    BID_t mining_on = 0;
    EID_t mining_event = 0;

    // ~Peer(){
    //     links.clear();
    //     log("Peer::desctuctor");
    // }
};

void ConnectGraphByRandomWalk(std::vector<Peer> &);

#endif