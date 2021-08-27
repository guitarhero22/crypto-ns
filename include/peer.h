#ifndef __PEER_H__
#define __PEER_H__

#include<random>
#include<unordered_map>
#include<vector>

#include "event.h"
#include "utils.h"

class Peer; //forward Declaration

/**
 * Messages will live in the link untill they reach the target
 * Make methods so that link can send/receive transactions to/from target/source
 */ 
class Link{
    public:
        Peer *src;
        Peer *tgt;
};

/**
 * ! Mention in report why exponential distro is used for Txns
 * ! For a loop-less transaction forwarding, remember all the transactions that have been sent out, 
 * ! dont send them out again and send them out only to the peers who did not send you that transaction
 */ 
class Peer {
    public: 
        Peer(){};
        Peer(double tx);

        uint ID;
        std::mt19937 rng;
        std::exponential_distribution<> TxTimeDist; //For sampling time intervals between transactions
        bool is_slow;
        std::unordered_map<Peer*, Link> links;

    protected:
};


void ConnectGraphByRandomWalk(std::vector<Peer>&);

#endif