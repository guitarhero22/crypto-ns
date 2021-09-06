#include "simulator.h"
#include <iostream>
#include <queue>
#include <vector>
#include <stack>

// extern int NUM_PEERS;

Simulator::Simulator(ID_t n) : num_peers(n)
{
    NUM_PEERS = n;
    peers = std::vector<Peer>(n);

    for(int i=0; i < n; ++i) peers[i] = Peer(i, 10, 100, (i%2));
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
            break;
        };
    }

    for(auto peer: peers){
        log(tos(peer.tree.blks.size()));
    }

    log("Ending Simulation...");
}

int Simulator::P2P2dot(std::ofstream &file){

    if(!file.is_open()) return -1;

    file << "graph {\n";

    std::vector<bool> vis(num_peers, false);

    for(Peer p: peers){
        std::string color = p.slow ? "red" : "blue";
        file << p.ID << " [shape=doublecircle, color="+color+"]\n" ;
    }

    std::stack<int> Q;
    if(peers.size() > 0) Q.push(0), vis[0] = true;

    while(!Q.empty()){
        int p = Q.top();
        Q.pop();

        for(auto n : peers[p].links){
            if(vis[n.first -> ID]) continue;
            file << p << " -- " << n.first -> ID << "\n";
            vis[n.first -> ID] = true;
            Q.push(n.first -> ID);
        }
    }

    file << "}";

    return 0;
}

int Simulator::trees2dot(std::string basename){
    
    for(auto p : peers){
        std::ofstream file(basename + std::to_string(p.ID) + ".dot");
        p.tree._2dot(file);
    }

    return 0;
}