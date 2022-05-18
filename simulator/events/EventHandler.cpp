#include "EventHandler.h"

void EventHandler::loadEvents(string filename) {
    ifstream input;
    string line;
    input.open(filename);
    //getline(input, line); // ignore the first line as those topology attributes are not handled by the Event handler
    int garbage;
    input >> garbage >> garbage >> garbage;

    int eventType;
    while (input >> eventType) {
        cout << line << endl;
        // stringstream lineStream(line);
        // lineStream >> eventType;
        Event* event = new Event();
        event->eventType = (Event::EventType) eventType;
        event->duration = EVENT_DURATION_DEFAULT;

        switch (eventType) {
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
        // cout << "Event: " << event->
        events.push(event);
    }
}

Event* EventHandler::nextEvent() {
    if (!events.empty()) {
        Event* event = events.front();
        events.pop();

        return event;
    }
    else return nullptr;
}