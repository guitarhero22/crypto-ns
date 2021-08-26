#ifndef __PEER_H__
#define __PEER_H__

#include<random>

/**
 * ! Mention in report why exponential distro is used for Txns
 */ 
class peer {
    public: 
        peer(double tx);

        uint ID;
        std::mt19937 rng;
        std::exponential_distribution<> TxTimeDist; //For sampling time intervals between transactions
        bool is_slow;

    protected:
};

#endif