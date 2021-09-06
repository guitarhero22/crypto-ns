#include "simulator.h"
#include <iostream>
#include <queue>
#include <vector>
#include <stack>

// extern int NUM_PEERS;

Simulator::Simulator(ID_t n, float _z, Ticks Tx, std::vector<Ticks> &meanTk) : num_peers(n), z(_z)
{
    NUM_PEERS = n;
    peers = std::vector<Peer>(n);

    for (int i = 0; i < n; ++i)
        peers[i] = Peer(i, Tx, meanTk[i], i + 1 <= std::ceil(z * num_peers / 100.0));
    ConnectGraphByRandomWalk(peers);

    auto peerInitEvents = Peer::INIT(peers);
    for (auto &e : peerInitEvents)
    {
        if (e == NULL)
            continue;
        eventQ.push(e);
    }

    
}

void Simulator::start(Ticks end_time)
{
    auto milestone = new Milestone();

    //Putting Milestones
    for(int i = 0; i < 10; ++i){
        eventQ.push(new Event((i + 1) * end_time / 10, milestone, reinterpret_cast<callback_t>(&Milestone::plant)));
    }

    log("Starting Simulation...\n");
    while (!eventQ.empty())
    {
        auto nexus = eventQ.top();
        eventQ.pop();

        if (nexus->timestamp <= end_time)
        {
            auto children = nexus->execute(nexus->timestamp);
            for (auto &child : children)
            {
                if (child->timestamp < end_time)
                    eventQ.push(child);
            }
        }
        else
        {
            break;
        };
    }
    log("\n\nSimulation Complete...\n");
}

int Simulator::P2P2dot(std::ofstream &file)
{

    if (!file.is_open())
        return -1;

    file << "graph {\n";

    std::vector<bool> vis(num_peers, false);

    for (Peer p : peers)
    {
        std::string color = p.slow ? "red" : "blue";
        file << p.ID << " [shape=doublecircle, color=" + color + "]\n";
    }

    std::stack<int> Q;
    if (peers.size() > 0)
        Q.push(0), vis[0] = true;

    while (!Q.empty())
    {
        int p = Q.top();
        Q.pop();

        for (auto n : peers[p].links)
        {
            if (vis[n.first->ID])
                continue;
            file << p << " -- " << n.first->ID << "\n";
            vis[n.first->ID] = true;
            Q.push(n.first->ID);
        }
    }

    file << "}";

    return 0;
}

int Simulator::trees2dot(std::string basename)
{
    for (auto p : peers)
    {
        std::ofstream file(basename + std::to_string(p.ID) + ".dot");
        p.tree._2dot(file, p.ID);
    }

    return 0;
}

int Simulator::peerDump(std::string basename)
{
    for (auto p : peers)
    {
        std::ofstream file(basename + std::to_string(p.ID) + ".dump");
        p.tree._2dump(file, p.ID);
    }

    return 0;
}

int Simulator::resultDump(std::string basename){
    std::ofstream file(basename + "results.dump");

   file << "\nResults:\n";
    BID_t tot = 0;

    file << "Peer ID: <peer's blocks> <% of total blocks\n>";
    BID_t chainL = 1;
    for(auto peer: peers){
        if(peer.tree.longest != NULL)
            chainL = std::max(chainL, peer.tree.longest -> chainLength);
   }

    for(auto peer : peers){
        file << peer.ID << ": " << peer.tree.blksByMe << " " << peer.tree.blksByMe * 100.0 / chainL;
        tot += peer.tree.blksByMe;
    }

    file << "\nTotal Blocks: " << tot;

    return 0;

}