#include "EventHandler.h"

void EventHandler::loadEvents(string filename) {
    ifstream input;
    input.open(filename);

    int garbage;
    input >> garbage >> garbage >> garbage; // Ignore the first line, as those topology attributes are not handled by the Event handler

    int eventType;
    while (input >> eventType) { // As long as we still have events
        Event* event = new Event();
        event->eventType = (Event::EventType) eventType;
        event->duration = EVENT_DURATION_DEFAULT;

        switch (eventType) { // Check event type
            case Event::SEND:
                input >> event->senderId;
                input >> event->receiverId;
                input >> event->bytes;
                break;
            case Event::JOIN:
                input >> event->x;
                input >> event->y;
                event->duration = EVENT_DURATION_JOIN;
                break;
            case Event::DISCONNECT:
                input >> event->hostId;
                break;
            case Event::MOVE:
                input >> event->hostId;
                input >> event->x;
                input >> event->y;
                break;
        }
        events.push(event); // Add the list to the waiting queue of events
    }
}

Event* EventHandler::nextEvent() {
    if (!events.empty()) { // If there are events left to be processed
        Event* event = events.front();
        events.pop();

        return event; // Return the next one in the queue
    }
    else return nullptr;
}