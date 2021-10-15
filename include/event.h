#ifndef __EVENT_H__
#define __EVENT_H__

#include "utils.h"
#include <vector>
// Define class Event here
// And inherit this class to form other events like send/receive/PoW
// An event will have a callback to invoke at the time event has to be executed

class Event; //Forward declaration

/**
 * EventGenerator to allow callbacks to member functions of Peers and Links,
 * Acts as Context for these callback functions
 */
class EventGenerator
{
public:
};

typedef std::vector<Event *> (EventGenerator::*callback_t)(Ticks, EID_t); ///< callback Type

/**
 * Event 
 * 
 * And Event can be added to the Event Queue and Executed using callbacks from EventGenerator Classes
 * Executing an Event generates more Events which can be added to the Event Queue
 */
class Event
{
public:
    static EID_t NUM_EVENTS;                            ///< To keep a count of number of Events
    static EID_t getNxtEvent() { return NUM_EVENTS++; } ///< Returns next available Event ID (EID_t)

    Event(){};                       ///< Constructor
    Event(Ticks t) : timestamp(t){}; ///< Constructor

    /**
     * Constructor
     *  
     * @param ts The time when this event is to be executed 
     * @param cxt The context to be used (The EventGenerator Obj) to call _play
     * @param cb A member function pointer to the class EventGenerator with the signature as in `callback_t`
     * 
     * @returns 
     */
    Event(Ticks ts, EventGenerator *cxt, callback_t cb) : timestamp(ts), context(cxt), callback(cb), ID(getNxtEvent()){};

    /**
     * Execute Event
     * 
     * Just a wrapper around, the callback of type `callback_t`,
     * which is called using the context (just a pointed to an EventGenerator Class)
     */
    std::vector<Event *> execute(Ticks t)
    {
        return ((*context).*callback)(t, ID);
        context = NULL;
    };

    EID_t ID;
    EventGenerator *context = NULL; ///< Context for the Callback
    callback_t callback;            ///<
    Ticks timestamp;                ///< time when event needs to be executed
};

/**
 * milestone for marking important times
 */
class Milestone : public EventGenerator
{
public:
    /**
     * @param _time milestone time
     * @param eid Event ID
     * 
     * @returns Events generated
     */
    std::vector<Event *> plant(Ticks _time, EID_t eid)
    {
        // std::cout << "." << std::flush;
        return std::vector<Event *>();
    };
};

/**
 * For sorting the Events, increasing timestamps in the Event Queue
 */
class compare_event
{
public:
    bool operator()(Event *a, Event *b)
    {
        return a->timestamp > b->timestamp;
    }
};

#endif