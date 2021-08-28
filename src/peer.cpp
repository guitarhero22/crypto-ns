#include <set>
#include "peer.h"

// Link
Link::Link(Peer *_src, Peer *_tgt): src(_src), tgt(_tgt)
{
    p = _unif_real<Ticks>(10, 500);
    c = (src->is_slow() || tgt->is_slow()) ? 5 : 100;
    rng = std::mt19937((std::random_device())());
    d = std::exponential_distribution<Ticks>(c / 96.0);
}

Event *Link::sendTxn(Txn txn, Ticks sendTime)
{
    if(src == NULL || tgt == NULL) {
        std::cerr << "Link not initialized properly" << std::endl;
    }
    Ticks delay = 0;
    delay += p;
    delay += txn.size / c;
    delay += d(rng);
    std::cout << src -> ID << " sending to " << tgt -> ID << " will reach at " << sendTime + delay << std::endl;
    return new PeerRcvTxn(src, tgt, txn, sendTime + delay);
}

// Peer
Peer::Peer(ID_t id, Ticks txnMean, Ticks temp, bool _slow) : slow(_slow), ID(id)
{
    rng = std::mt19937((std::random_device())());
    nextTxnTime = std::exponential_distribution<Ticks>(1/txnMean);
}

std::vector<Event *> Peer::rcvTxn(Peer *src, Txn txn, Ticks rcvTime)
{
    if(src != this)
    std::cout << "Txn Recvd by " << this -> ID << " at " << rcvTime << std::endl;
    std::vector<Event *> ret(0);
    if (txnSent.find(txn.ID) != txnSent.end())
        return ret;

    txnSent[txn.ID] = txn;
    txnPool[txn.ID] = txn;

    for (auto &l : links)
    {
        if (src != NULL && l.first->ID == src->ID)
            continue;
        ret.push_back(l.second.sendTxn(txn, rcvTime));
    }

    return ret;
}

std::vector<Event *> Peer::genTxn(Ticks genTime)
{

    // Add extra logic for Txn Generation
    Txn txn = Txn(0, 0, 50, genTime);

    std::cout << "Txn "<< txn.ID << " Generated by " << this -> ID << " at " << genTime  << std::endl;
    auto ret = this -> rcvTxn(this, txn, genTime);
    // ret.push_back(new PeerGenTxn(this, genTime + nextTxnTime(rng)));

    return ret;
}

// Events
std::vector<Event *> PeerRcvTxn::callback(Ticks t)
{
    return to->rcvTxn(from, txn, t);
}

std::vector<Event *> PeerGenTxn::callback(Ticks t)
{   
    return peer->genTxn(timestamp);
}

void ConnectGraphByRandomWalk(std::vector<Peer> &peers)
{
    std::set<Peer *> peer_set;

    unsigned int n = peers.size();
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