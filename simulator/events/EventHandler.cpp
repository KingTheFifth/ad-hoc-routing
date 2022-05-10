#include "EventHandler.h"

void EventHandler::loadEvents(string filename) {
    ifstream input;
    input.open(filename);

    int eventType;
    while (input >> eventType) {
        Event* event = new Event();
        event->eventType = (Event::EventType) eventType;
        event->duration = EVENT_DURATION_DEFAULT; // do this better
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

// void EventHandler::loadEventSendHost(ifstream input, Event* event) {
//     input >> event->senderId;
//     input >> event->receiverId;
//     input >> event->bytes;
// }
// void EventHandler::loadEventSendPos(ifstream input, Event* event) {
//     input >> event->senderId;
//     input >> event->x;
//     input >> event->y;
//     input >> event->bytes;
// }
// void EventHandler::loadEventJoin(ifstream input, Event* event) {
//     input >> event->x;
//     input >> event->y;
// }
// void EventHandler::loadEventDisconnect(ifstream input, Event* event) {
//     input >> event->hostId;
// }
// void EventHandler::loadEventMove(ifstream input, Event* event) {
//     input >> event->x;
//     input >> event->y;
// }