#ifndef EVENT_H
#define EVENT_H

struct Event {
    enum EventType {SEND, JOIN, DISCONNECT, MOVE};
    EventType eventType;
    int duration;

    // specific to SEND event
    unsigned senderId;
    unsigned receiverId;
    int bytes;

    double x;
    double y;

    unsigned hostId;

};

#endif