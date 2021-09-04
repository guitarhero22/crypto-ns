#include "simulator.h"
#include<iostream>
#include<queue>

// extern int NUM_PEERS;

Simulator::Simulator(ID_t n) 
{
    NUM_PEERS = n;
    peers = std::vector<Peer>(n);

    for(int i=0; i < n; ++i) peers[i] = Peer(i, 100, 400, (i%2));
    ConnectGraphByRandomWalk(peers);

    auto peerInitEvents = Peer::INIT(peers);
    for(auto &e : peerInitEvents) {
        if(e == NULL) continue;
        eventQ.push(e);
    }
}

void Simulator::start(Ticks end_time){

    log("Starting Simulation...");

    while(!eventQ.empty()){
        auto nexus = eventQ.top();
        eventQ.pop();

        if(nexus -> timestamp <= end_time){
            auto children = nexus -> execute(nexus -> timestamp);
            for(auto &child : children){
                if(child -> timestamp < end_time) eventQ.push(child);
            }
        }else {
            log(std::to_string(nexus->timestamp));
        };
    }

    log("Ending Simulation...");
}