#include "packet.h"
#include "host/host.h"

Packet::Packet(const Host* _source, const Host* _destination, int _time, int _id)
    : source(_source), destination(_destination), timeSent(_time), id(_id) {}

Packet::Packet(const Packet& other) {
    copyOther(other);
}

void Packet::copyOther(const Packet& other) {
    source = other.source;
    destination = other.destination;
    nextHop = other.nextHop;
    size = other.size;
    color = other.color;
    id = other.id;
}

void operator delete(void * p) {}
