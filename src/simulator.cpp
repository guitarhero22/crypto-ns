#include "simulator.h"
#include <iostream>
#include <queue>
#include <vector>
#include <stack>

// extern int NUM_PEERS;

void Simulator::setup(ID_t n, float _z, Ticks _Tx, std::vector<Ticks> &_meanTk, ID_t m)
{
    NUM_PEERS = n;
    num_peers = n;
    z = _z;
    Tx = _Tx;
    meanTk = _meanTk;
    peers = std::vector<Peer>(n);

    for (int i = 0; i < n; ++i)
        peers[i] = Peer(i, Tx, meanTk[i], false);

    log("\nChoosing slow nodes...");
    chooseSlowNodes();

    log("\nBuilding Network...");
    ConnectGraphByBarbasiAlbert(num_peers, m);

    log("\nSeeding Initial Events...");
    auto peerInitEvents = Peer::INIT(peers);
    for (auto &e : peerInitEvents)
    {
        if (e == NULL)
            continue;
        eventQ.push(e);
    }
}

void Simulator::setupSelfishMining(ID_t n, double _z, Ticks _Tx, Ticks _meanTk, std::vector<Ticks> &computePower, ID_t m)
{
    NUM_PEERS = n;
    num_peers = n;
    peers = std::vector<Peer>(n); 
    z = _z;
    Tx = _Tx;

    //Sanity check
    if(n != computePower.size()){
        logerr("Simulator::setupSelfishMining conflict sizes of computePower and num_peers");
    }

    Ticks total_compute_power = std::accumulate(computePower.begin(), computePower.end(), 0, [](const Ticks &t1, const Ticks &t2)
                                                { return t1 + t2; });

    // Calculate Mean Tk for each peer 
    for(int i = 0; i < n; ++i){
        if(computePower[i] <= 0) logerr("setupSelfishMining:: computePower cannot be zero");
        meanTk.push_back(_meanTk * total_compute_power / computePower[i]);
    }

    //Initialize peers
    for (int i = 0; i < n; ++i)
        peers[i] = Peer(i, Tx, meanTk[i], false);

    log("\nChoosing slow nodes...");
    chooseSlowNodes();

    log("\nBuilding Network...");
    ConnectGraphByBarbasiAlbert(num_peers, m);

    log("\nSeeding Initial Events...");
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
    for (int i = 0; i < 10; ++i)
    {
        eventQ.push(new Event((i + 1) * end_time / 10, milestone, reinterpret_cast<callback_t>(&Milestone::plant)));
    }

    log("\nStarting Simulation...\n");
    while (!eventQ.empty())
    {
        auto nexus = eventQ.top(); //event on top of heap
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

void Simulator::chooseSlowNodes()
{
    int num_slow_nodes = std::ceil(z * num_peers / 100);
    if (num_slow_nodes == 0)
        return;
    int m = num_peers / num_slow_nodes;

    std::mt19937 rng((std::random_device())());
    std::uniform_int_distribution<unsigned int> select(0, num_peers - 1);

    std::set<int> st;
    while (st.size() < num_slow_nodes)
    {
        int k = select(rng);
        st.insert(k);
    }

    for (int k : st)
        peers[k].slow = true;

    // for(int i = 0; i < num_peers; ++i){
    //     if(i % m == 0)
    //         peers[i].slow = true;
    // }
    return;
}

void Simulator::ConnectGraphByBarbasiAlbert(ID_t n, ID_t m)
{
    if (num_peers < 2 || m < 2)
        return;

    ConnectGraphByRandomWalk(m);

    std::vector<ID_t> deg(n);

    for (int i = 0; i < m; ++i)
    { // Initialize a connected network with m nodes
        deg[i] = peers[i].links.size();
    }

    std::mt19937 rng((std::random_device())());

    for (int i = m; i < n; ++i)
    {
        std::discrete_distribution<ID_t> select(deg.begin(), deg.begin() + i);

        for (int j = 0; j < m; ++j)
        { // Connect this one to m nodes
            int k = select(rng);
            peers[i].links[&peers[k]] = Link(&peers[i], &peers[k]);
            peers[k].links[&peers[i]] = Link(&peers[k], &peers[i]);
        }

        for (int j = 0; j < i + 1; ++j)
        {
            deg[j] = peers[j].links.size();
        }
    }

    return;
}

void Simulator::ConnectGraphByRandomWalk(ID_t n)
{
    std::set<Peer *> peer_set;

    for (int i = 0; i < n; ++i)
        peer_set.insert(&peers[i]);

    std::mt19937 rng((std::random_device())());
    std::uniform_int_distribution<unsigned int> select(0, n - 1);

    unsigned int current = 0;
    peer_set.erase(&peers[current]);
    while (!peer_set.empty())
    {

        unsigned int next = select(rng);

        if (current == next)
            continue;

        auto &nbrs = peers[current].links;
        if (nbrs.find(&peers[next]) != nbrs.end())
        {
            current = next;
            continue;
        }

        peer_set.erase(&peers[next]);
        nbrs[&peers[next]] = Link(&peers[current], &peers[next]);
        peers[next].links[&peers[current]] = Link(&peers[next], &peers[current]);

        current = next;
    }

    return;
}

int Simulator::P2P2dot(std::ofstream &file)
{

    if (!file.is_open())
        return -1;

    file << "graph {\nlayout=\"twopi\"\n";

    std::vector<bool> vis(num_peers, false);

    for (Peer p : peers)
    {
        std::string color = p.slow ? "red" : "blue";
        file << p.ID << " [color=" + color + "]\n";
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

int Simulator::resultDump(std::string basename)
{
    std::ofstream file(basename + "results.dump");

    // file << "\nResults:\n";
    BID_t tot = 0;

    file << "Peer ID,<ComutePower>,<peer's blocks>,<in longest chain>,<% in longest chain>,<orphans received>\n";

    Ticks total_power = 0;
    for (auto &p : meanTk)
        total_power += 1 / p;

    for (int i = 0; i < num_peers; ++i)
    {
        auto &peer = peers[i];
        file << peer.ID
             << "," << 100.0 / (total_power * meanTk[i])
             << "," << peer.tree.blksByMe
             << "," << peer.tree.blocksInChainById(peer.ID)
             << "," << peer.tree.blocksInChainById(peer.ID) * 100.0 / peer.tree.longest->chainLength
             << "," << peer.tree.orphansRcvd
             << "\n";
        tot += peer.tree.blksByMe;
    }

    // file << "\nTotal Blocks in Trees: " << tot << std::endl;
    // file << "\nTotal Blocks Generated: " << tot;

    return 0;
}