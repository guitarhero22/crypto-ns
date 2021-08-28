#ifndef __EVENT_H__
#define __EVENT_H__

#include "utils.h"
#include <vector>
// Define class Event here
// And inherit this class to form other events like send/receive/PoW
// An event will have a callback to invoke at the time event has to be executed

class Event{
    public:
        Event(){};
        Event(Ticks t):timestamp(t){};
        virtual std::vector<Event*> callback(Ticks) = 0;
        Ticks timestamp; 
};

class compare_event{
    public: 
        bool operator() (Event *a, Event *b){
            return a -> timestamp > b -> timestamp;
        }
};

#endif