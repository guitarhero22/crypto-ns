/** @file simulator.h
 * 
 * Contains the Simulator Class
 * 
 */
#ifndef __SIMULATOR_H__
#define __SIMULATOR_H__

#include "event.h"
#include "peer.h"
#include "block.h"
#include <vector>
#include <queue>
#include <fstream>

//Simulator class which will have a single event queue/heap/set for all the events and will execute these events one by one

/** 
 * @class Simulator
 * 
 * Simulator class initializes and launches the simulation. 
 * It contains an EventQueue, which is a heap data strucutre, 
 * so that the event with the least timestamp can be executed next
 * 
 */
class Simulator
{
public:
  Simulator(){};

  /**
     * Setup the Simulator.
     * 
     * Initializes some important structures and builds set up for the Simulation
     * 
     * @param n Number of peers to simulate
     * @param z % of nodes slow
     * @param Tx mean interarrival time of TxnscA
     * @param meanTk vector of mean of interarrival time of blocks for each node
     * @param m for Barabasi-Albert Algorithm
     * 
     * @returns Instance of the Simulator Class
     */
  void setup(ID_t n, float z, Ticks Tx, std::vector<Ticks> &meanTk, ID_t m);

  /**
     * Setup the Simulator.
     * 
     * Initializes some important structures and builds set up for the Simulation
     * 
     * @param n Number of peers to simulate
     * @param z % of nodes slow
     * @param Tx mean interarrival time of TxnscA
     * @param meanTk mean of interarrival of blocks in the network
     * @param computePower compute power for each node
     * @param m for Barabasi-Albert Algorithm
     * 
     * @returns Instance of the Simulator Class
     */
  void setupAdversarialMining(ID_t n, double z, Ticks Tx, Ticks meanTk, std::vector<Ticks> &ComputePower, ID_t m, double _num_connections, std::string adversary);

  /**
     * Start the Simulation
     * 
     * This function continuously picks element from the Event Queue, and calls the `callback` method in the Event.
     * Moreover, all the new events that will be generated by these Events are inserted into the EventQueue.
     * 
     * This will be done for time < endTime 
     * 
     * @param endTime When to end the simulation
     * @return Nothing
     */
  void start(Ticks endTime); // Start Simulation

  /**
     * To Sample a Scale Free Newtork
     * 
     * @param Number of peers to consider
     * @param m initial set size
     */
  void ConnectGraphByBarbasiAlbert(ID_t n, ID_t m = 2);

  /**
     * To build the Netwoek Topology 
     * 
     * @param Number of peers to consider
     */
  void ConnectGraphByRandomWalk(ID_t);

  /**
     * Choose Slow Nodes
     */
  void chooseSlowNodes();

  /**
     * Print the p2p Network to a dot file
     * 
     * @param file output file stream
     * @returns status
     */
  int P2P2dot(std::ofstream &file);

  /**
     * Print all the trees to a dot file
     * 
     * @param basename basename for output
     * @returns status
     */
  int trees2dot(std::string basename);

  /**
     * Print all the trees to a dump file
     * 
     * @param basename basename for output
     * @returns status
     */
  int peerDump(std::string basename);

  /**
     * Prints other results
     */
  int resultDump(std::string basename);

  /**
     * Event Queue.
     * 
     * A heap data structure, so that the event at the top always has the smallest timestamp
     */
  std::priority_queue<Event *, std::vector<Event *>, compare_event> eventQ;

  BID_t num_peers = 0;     // Number of peer
  std::vector<Peer> peers; // Vector Holding the peers
  std::vector<Ticks> meanTk;
  Ticks Tx = 0;
  float z = 0;
};

#endif