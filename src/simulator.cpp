#include "simulator.h"
#include<iostream>
#include<queue>

extern int NUM_PEERS;

Simulator::Simulator(unsigned int n) 
{
    NUM_PEERS = n;
    peers = std::vector<Peer>(n);

    for(int i=0; i < n; ++i) peers[i] = Peer(i, 500, 0, (i%2));
    ConnectGraphByRandomWalk(peers);

    for(int i=0; i < n; ++i) {
        eventQ.push(new PeerGenTxn(&peers[i], peers[i].nextTxnTime(peers[i].rng)));
    }
}

void Simulator::start(Ticks end_time){
    while(!eventQ.empty()){
        auto nexus = eventQ.top();
        eventQ.pop();

        if(nexus -> timestamp <= end_time){
            auto children = nexus -> callback(nexus -> timestamp);
            for(auto &child : children){
                if(child -> timestamp < end_time) eventQ.push(child);
            }
        }else break;
    }
}