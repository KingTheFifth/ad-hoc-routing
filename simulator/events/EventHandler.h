#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include "constants.h"
#include "Event.h"
#include <queue>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

class EventHandler {
    public:
        void loadEvents(string filename);

        /**
         * Returns the next event. Returns nullptr if there are no more scheduled
         * events. Note that the user must delete the Event pointer after use.
         */
        Event* nextEvent();


    private:
        queue<Event*> events;

};

#endif
