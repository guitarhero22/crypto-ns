#include<iostream>
#include<string>
#include "simulator.h"

/**
 * * Inputs: 
 *  N: Number of Nodes
 *  Z: % of nodes slow 
 */
int main(int argc, char* argv[]){

    if(argc < 2){
        std::cout << "Usage:\n$ ./crypto-ns <num_peers>\n";
        return 0;
    }

    unsigned int num_peers = std::stoul(std::string(argv[1]));
    Simulator s(num_peers);
}