#ifndef __EVENT_H__
#define __EVENT_H__

#include "utils.h"
#include <vector>
// Define class Event here
// And inherit this class to form other events like send/receive/PoW
// An event will have a callback to invoke at the time event has to be executed

class Event; //Forward declaration
class EventGenerator{};
typedef std::vector<Event*> (EventGenerator::*callback_t)(Ticks, EID_t);

class Event{
    public:
        static EID_t NUM_EVENTS;
        static EID_t get_next_event(){return NUM_EVENTS++;}

        Event(){};
        Event(Ticks t):timestamp(t){};
        Event(Ticks t, EventGenerator* o, callback_t _play) : timestamp(t), obj(o), play(_play), ID(get_next_event()){};
        EID_t ID;
        EventGenerator* obj;
        callback_t play;
        std::vector<Event*> callback(Ticks t) {
            return ((*obj).*play)(t, ID);
        };
        Ticks timestamp; 
};

class compare_event{
    public: 
        bool operator() (Event *a, Event *b){
            return a -> timestamp > b -> timestamp;
        }
};

#endif