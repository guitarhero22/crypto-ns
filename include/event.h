#ifndef __EVENT_H__
#define __EVENT_H__

#include "utils.h"
#include <vector>
// Define class Event here
// And inherit this class to form other events like send/receive/PoW
// An event will have a callback to invoke at the time event has to be executed

class Event; //Forward declaration
class EventGenerator{};
typedef std::vector<Event*> (EventGenerator::*callback_t)(Ticks);

class Event{
    public:
        Event(){};
        Event(Ticks t):timestamp(t){};
        Event(Ticks t, EventGenerator* o, std::vector<Event*> (EventGenerator::*_play)(Ticks)) : timestamp(t), obj(o), play(_play){};
        EventGenerator* obj;
        std::vector<Event*> (EventGenerator::*play)(Ticks);
        std::vector<Event*> callback(Ticks t) {
            ((*obj).*play)(t);
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