#include "packet.h"
#include "host/host.h"

Packet::Packet(const Host* _source, const Host* _destination, int _time)
    : source(_source), destination(_destination), timeSent(_time) {}

Packet::Packet(const Packet& other) {
    copyOther(other);
}

void Packet::copyOther(const Packet& other) {
    source = other.source;
    destination = other.destination;
    nextHop = other.nextHop;
    size = other.size;
}