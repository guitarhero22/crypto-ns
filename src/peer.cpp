#include <set>
#include "peer.h"

// Link
Link::Link(Peer *_src, Peer *_tgt) : src(_src), tgt(_tgt)
{
    p = _unif_real<Ticks>(10, 500); // speed of light delay
    c = (src->is_slow() || tgt->is_slow()) ? 5 : 100; // link speed
    // rng = std::mt19937((std::random_device())());  //random generator object
    d = std::exponential_distribution<Ticks>(c / 96.0); //distribution for queuing delay
}

Event *Link::send(Msg *msg, Ticks sendTime)
{
    if (src == NULL || tgt == NULL)
        logerr("Link not initialized properly");

    Ticks delay = 0;
    delay += p; //speed of light delay
    delay += msg->size / c;//message size
    delay += d(rng); //sample queuing delay
    Ticks aod = sendTime + delay; //time of arrival

    Q.push(pTM({aod, msg})); // push to queue to send later

    return new Event(aod, this, reinterpret_cast<callback_t>(&Link::sent));
}

std::vector<Event *> Link::sent(Ticks sentTime, EID_t eid)
{

    if (Q.empty()) //sanity check, no messages to send, still sent was called
        logerr("Link::sent msg Queue Empty");

    auto msg = Q.top(); //find the message that reaches next

    if (std::abs(msg.first - sentTime) > Ticks(1e-6)) //sanity check
        logerr("Link::sent sentTime not equal to message time: MSG_TYPE = " + std::to_string(msg.second->type) + " time in msg=" + std::to_string(msg.first) + " sentTime=" + std::to_string(sentTime));

    Q.pop(); //Done

    return (*tgt).rcvMsg(src, msg.second, sentTime); //send the message to tgt peer
}

// Peer
std::vector<Event *> Peer::INIT(std::vector<Peer> &peers)
{
    int sz = peers.size();
    std::vector<Event *> initEvents(2 * sz, NULL); //seed events
    for (int i = 0; i < sz; ++i)
    {
        //seed event for transactions
        initEvents[i] = new Event(peers[i].nextTxnTime(rng), &peers[i], reinterpret_cast<callback_t>(&Peer::genTxn));

        //seed events for mining
        initEvents[i + sz] = peers[i].start_mining_session(0);
    }

    log("all seeded, tree being initialized");

    Tree::INIT();
    log("Peers Initialized");

    return initEvents;
};

Peer::Peer(ID_t id, Ticks txnMean, Ticks computePower, bool _slow) : slow(_slow), ID(id)
{   
    tree = new Tree(id);

    // rng = std::mt19937((std::random_device())());
    nextTxnTime = std::exponential_distribution<Ticks>(1 / txnMean);
    nextBlkTime = std::exponential_distribution<Ticks>(1 / computePower);
}
Peer::Peer(ID_t id, Ticks txnMean, Ticks computePower, bool _slow, std::string _adversary) : slow(_slow), ID(id), adversary(_adversary)
{
    if(adversary == "selfish"){
        log(tos(id) + " Assigned as selfish");
        tree = new SelfishTree(id);
    }else{
        if(adversary == "stubborn"){
            log(tos(id) + " Assigned as stubborn");

            tree = new StubbornTree(id);
        }
        else{
            if(adversary == "not"){
                log(tos(id) + " Assigned as normal");
                tree = new Tree(id);
            }
            else
            throw NotImplementedException(adversary + " has not been implemented");
        }
    }
    // rng = std::mt19937((std::random_device())());
    nextTxnTime = std::exponential_distribution<Ticks>(1 / txnMean);
    nextBlkTime = std::exponential_distribution<Ticks>(1 / computePower);
}

std::vector<Event *> Peer::rcvMsg(Peer *src, Msg *msg, Ticks rcvTime)
{
    std::vector<Event *> ret;

    //check type of the message and call appropriate function
    if (msg->type == BLOCK)
    {
        ret = rcvBlk(src, static_cast<Blk *>(msg), rcvTime);
    }
    else if (msg->type == TXN)
    {
        ret = rcvTxn(src, static_cast<Txn *>(msg), rcvTime);
    }
    else
        logerr("Peer::rcvMsg msg Type none of BLOCK or TXN"); //sanity check

    //return new events generated
    return ret;
}

//Peer Receivs Txn
std::vector<Event *> Peer::rcvTxn(Peer *src, Txn *txn, Ticks rcvTime)
{
    /**
     * src: the one who sent this message
     */

    std::vector<Event *> ret;

    //check if transaction is already present
    if (!tree -> addTxn(txn))
        // no need to send to peers, so return
        return ret;

    //txn not yet seen, so send to peers
    for (auto &l : links)
    {
        if (src != NULL && l.first->ID == src->ID)
            //don't send the message to the sender of the message
            continue;
        ret.push_back(l.second.send(txn, rcvTime));
    }

    //return new events generated
    return ret;
}

//Peer Generates Transactions
std::vector<Event *> Peer::genTxn(Ticks genTime, EID_t eid)
{
    // ! Add Logic for selection of TXN Amount
    int send_to = _unif_int<ID_t>(0, NUM_PEERS - 1);
    /**
     * ID: the one who sends money
     * send_to: the one who receives money
     */
    Txn *txn = Txn::new_txn(ID, send_to, 50, genTime); 

    //Once a new transaction is generated, call rcvTxn to add it to tree and send to peers
    auto ret = rcvTxn(this, txn, genTime);

    //Add event for generation of next transaction
    ret.push_back(new Event(genTime + nextTxnTime(rng), this, reinterpret_cast<callback_t>(&Peer::genTxn)));
    
    //return all the new events generated
    return ret;
}

Event *Peer::start_mining_session(Ticks startTime)
{
    //Select the block to mine on
    mining_on = tree -> startMining(ID, startTime);

    //sample the time this block takes to mine
    Ticks genTime = startTime + nextBlkTime(rng);

    //generate the mining event
    callback_t cb = reinterpret_cast<callback_t>(&Peer::genBlk);
    Event *genBlkEvent = new Event(genTime, this, cb);
    mining_event = genBlkEvent->ID;

    //return the generated events
    return genBlkEvent;
};

std::vector<Event *> Peer::rcvBlk(Peer *src, Blk *blk, Ticks rcvTime)
{
    /**
     * src: the peer who sent this block to us
     */

    //container for new events generated
    std::vector<Event *> ret;

    //see if the block is already present in the tree
    auto blk_list = tree -> addBlk(blk, rcvTime); //blk_list is a pair of type (int, vector<blk*>)

    if (blk_list.first == DONT_SEND || blk_list.first == DONT_KNOW)
    {
        //if already present, no need to send to neighbours
        return ret;
    }

    //if new longest chain is discovered start a new mining session
    if (blk_list.first == NEW_LONGEST_CHAIN)
    {
        ret.push_back(start_mining_session(rcvTime)); //start new mining session
    }

    //send the block to all neighbours other than the sender to avoid loops
    for (auto &_blk : blk_list.second){
        for (auto &l : links)
        {
            if (src != NULL && l.first->ID == src->ID)
                continue;
            ret.push_back(l.second.send(_blk, rcvTime));
        }
    }
    // for (auto &l : links)
    // {
    //     if (src != NULL && l.first->ID == src->ID)
    //         continue;
    //     ret.push_back(l.second.send(blk, rcvTime));
    // }

    //return new events generated
    return ret;
}

//Peer Generates Block - event callback for ending of a mining session
std::vector<Event *> Peer::genBlk(Ticks genTime, EID_t eid)
{
    //new events generated
    std::vector<Event *> ret;

    //Check if this mining session was abandoned
    if (eid != mining_event)
    {
        // log("Mining already started for another block");
        return ret;
    }

    //Create New Block
    Blk *blk = tree -> mineBlk(ID, mining_on, genTime);

    //BroadCast new Block, rcvBlk will also create a new mining session
    ret = rcvBlk(this, blk, genTime);

    return ret;
}
