#include <fstream>
#include<iostream>
#include<string>
#include "simulator.h"
#include "utils.h"

/**
 * **Inputs**: 
 *  N: Number of Nodes
 *  Z: % of nodes slow 
 *  Tx: Mean inter arrival time of transactions
 * 
 */
int main(int argc, char* argv[]){

    if(argc < 2){
        std::cout << "Usage:\n$ ./crypto-ns <config_file_path>\n";
        return 0;
    }

    log("Configuration will be taken from file " + std::string(argv[1]));

    std::ifstream config;
    config.open(std::string(argv[1]));

    if(config.is_open()) 
        log("Configuring...");
    else
        logerr("Could Not Open Configuration File");

    ID_t numPeers, m;
    double z;
    Ticks Tx, endTime;
    
    config >> numPeers >> z >> Tx >> endTime >> m;

    std::vector<Ticks>meanTk(numPeers);

    log("\nThe configured options are:\n");
    log("NumPeers: " + tos(numPeers));
    log("z: " + tos(z));
    log("Tx: " + tos(Tx));
    log("endTime: " + tos(endTime));
    log("m: " + tos(m));

    log("");

    for(ID_t i = 0; i < numPeers; ++i) {
        config >> meanTk[i];
        log(tos(i) + " " + tos(meanTk[i]));
    }

    log("*************************\n");

    Simulator s(numPeers, z, Tx, meanTk, m);

    std::ofstream P2PDot("dots/P2P.dot");
    s.P2P2dot(P2PDot);
    P2PDot.close();

    s.start(endTime);

    s.trees2dot("dots/");
    s.peerDump("dumps/");
    s.resultDump("dumps/");

    return 0;
}