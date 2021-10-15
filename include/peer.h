/** @file peer.h
 * This file contains peers
 */

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

typedef std::pair<Ticks, Msg *> pTM;

class Peer; //forward Declaration

/**
 * One Way Link between Peers.
 * 
 * Messages will live in the link untill they reach the target
 */
class Link : public EventGenerator
{
public:
    Link() {} ///< Constructor

    /**
     * Link Constructor
     * 
     * @param src pointer to sender
     * @param tgt pointer to the receiver
     * 
     * @returns instance to a Link class
     */
    Link(Peer *src, Peer *tgt);

    /**
     * Send a message
     * 
     * @param msg Ponter to the message
     * @param sendTime time when sender clicks send on his machine
     */
    Event *send(Msg *msg, Ticks sendTime);

    /**
     * Callback, when transaction reaches the Target
     * 
     * @param reachTime time when the target reches the target
     * @param eid The Event ID which invoked the fuction
     * 
     * @returns the new events generated
     */
    std::vector<Event *> sent(Ticks reachTime, EID_t eid);

    /**
     * Heap Data Structure
     * 
     * To queue the messages 
     */
    std::priority_queue<pTM, std::vector<pTM>, std::greater<pTM>> Q;

    Peer *src;                              ///< Sender
    Peer *tgt;                              ///< Receiver
    Ticks p;                                ///< the parameter p
    Ticks c;                                ///< c speed of light delay
    // std::mt19937 rng;                       ///< Random number generator
    std::exponential_distribution<Ticks> d; ///< Exponential distribution for queuing delay
};

/**
 * ! Mention in report why exponential distro is used for Txns
 * ! For a loop-less transaction forwarding, remember all the transactions that have been sent out, 
 * ! dont send them out again and send them out only to the peers who did not send you that transaction
 */

/**
 * Peer
 * 
 * The main class that simulates the Miners
 */
class Peer : public EventGenerator
{
public:
    /**
     * Initialized the peers
     * 
     * @param peers container of the peers to initialize
     */
    static std::vector<Event *> INIT(std::vector<Peer> &peers);

    Peer(){}; ///< Constructor

    /**
     * 
     * Constructor
     * 
     * @param id ID
     * @param meanTxn mean interval between transactions
     * @param meanMin mean time between consecutive block
     * @param slow whether the peer has slow internet connection
     * 
     * @returns instance of the Peer Class
     */
    Peer(ID_t id, Ticks meanTxn, Ticks meanMine, bool slow);
    Peer(ID_t id, Ticks meanTxn, Ticks meanMine, bool slow, bool selfish);

    /**
     * Simple Getter
     */
    bool is_slow() { return slow; }

    /**
     * receive Message callback
     * 
     * Callback for when peer Has received a message
     * 
     * @param sender sender of the message 
     * @param msg the message
     * @param rcvTime time of receipt
     * 
     * @returns the new events generated
     */
    virtual std::vector<Event *> rcvMsg(Peer *sender, Msg *msg, Ticks rcvTime);

    /**
     * receive Transaction callback
     * 
     * Callback for when peer Has received a message
     * 
     * @param sender sender of the transaction 
     * @param txn the transaction
     * @param rcvTime time of receipt
     * 
     * @returns the new events generated
     */
    virtual std::vector<Event *> rcvTxn(Peer *sender, Txn *txn, Ticks rcvTime);

    /**
     * callback for when its time to generate the next transaction
     * 
     * @param genTime Timestamp to be put in the transaction
     * @param eid event ID that invoked this Peer
     * 
     * @returns the new events generated
     */
    virtual std::vector<Event *> genTxn(Ticks genTime, EID_t eid);

    /**
     * receive Block callback
     * 
     * Callback for when peer Has received a message
     * 
     * @param sender Sender of the Block
     * @param txn the transaction
     * @param rcvTime time of receipt
     * 
     * @returns the new events generated
     */
    virtual std::vector<Event *> rcvBlk(Peer *sender, Blk *, Ticks);

    /**
     * callback for when its time to generate the next block
     * 
     * @param genTime Timestamp to be put in the block
     * @param eid event ID that invoked this Peer
     * 
     * @returns the new events generated
     */
    virtual std::vector<Event *> genBlk(Ticks genTime, EID_t eid);

    /**
     * To Initialize some stuff for starting a new mining session
     * 
     * @returns a Event with the genBlk callback
     */
    virtual Event *start_mining_session(Ticks startTime);

    //for randomness
    ID_t ID;                                          ///< ID
    // std::mt19937 rng;                                 ///< Random Number Generator
    std::exponential_distribution<Ticks> nextTxnTime; ///< For sampling time intervals between Transactions
    std::exponential_distribution<Ticks> nextBlkTime; ///< For sampling time intervals between Blocks

    bool slow = false;          ///< Whether this peer has slow connetion
    bool selfish = false;       ///< Whether this peer is selfish
    Ticks computePower; ///< Compute Power, for deciding when a block will be mined
    Ticks txnGenFreq;   ///< Transaction Generation Frequency

    Tree* tree;                              ///< The tree
    std::unordered_map<Peer *, Link> links; ///< Neighbourhood information

    BID_t mining_on = 0;    ///< The last Block on which mining is in process
    EID_t mining_event = 0; ///< The event ID which will invoke this peer for mining the next block
};

class SelfishPeer: public Peer{
    public:
};

#endif