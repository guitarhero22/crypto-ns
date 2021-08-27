#include "simulator.h"
#include<iostream>

Simulator::Simulator(unsigned int n) 
{
    peers = std::vector<Peer>(n);

    for(int i=0; i < n; ++i) peers[i].ID = i;
    ConnectGraphByRandomWalk(peers);

    for(int i=0; i < n; ++i) {
        for(auto p : peers[i].links) std::cout << p.first -> ID << " ";
        std::cout << std::endl;
    }
}