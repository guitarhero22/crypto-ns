#include<set>
#include "peer.h"

void ConnectGraphByRandomWalk(std::vector<Peer> &peers){
    std::set<Peer*> peer_set;

    unsigned int n = peers.size();
    for(int i=0; i<n; ++i) peer_set.insert(&peers[i]);

    std::mt19937 rng((std::random_device())());
    std::uniform_int_distribution<unsigned int> select(0, n-1);

    unsigned int current = 0;
    peer_set.erase(&peers[current]);
    while(!peer_set.empty()){

        unsigned int next = select(rng);

        if(current == next) continue;

        auto &nbrs = peers[current].links;
        if(nbrs.find(&peers[next]) != nbrs.end())
        {
            current = next;
            continue;
        }

        peer_set.erase(&peers[next]);
        nbrs[&peers[next]] = Link();
        peers[next].links[&peers[current]] = Link();

        current = next;
    }

    return;
}