#ifndef EVENT_H
#define EVENT_H

struct Event {
    enum EventType {SEND, JOIN, DISCONNECT, MOVE};
    EventType eventType;
    int duration;

    double x;
    double y;

    unsigned hostId;

    // Specific to SEND event
    unsigned senderId;
    unsigned receiverId;
    int bytes;
    
};

#endif