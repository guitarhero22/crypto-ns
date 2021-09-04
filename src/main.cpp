#include<iostream>
#include<string>
#include "simulator.h"
#include "utils.h"

/**
 * * Inputs: 
 *  N: Number of Nodes
 *  Z: % of nodes slow 
 */
int main(int argc, char* argv[]){

    if(argc < 3){
        std::cout << "Usage:\n$ ./crypto-ns <num_peers> <sym_time>\n";
        return 0;
    }

    ID_t numPeers = std::stoul(std::string(argv[1]));
    Ticks endTime = std::stof(std::string(argv[2]));

    Simulator s(numPeers);
    s.start(endTime);

    log("End of main");
    return 0;
}