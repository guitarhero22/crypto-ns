#ifndef __SIMULATOR_H__
#define __SIMULATOR_H__

#include "event.h"
#include "peer.h"
#include<vector>

//Simulator class which will have a single event queue/heap/set for all the events and will execute these events one by one
class Simulator{
    public:
        Simulator(unsigned int n);
        std::vector<Peer> peers;
};

#endif