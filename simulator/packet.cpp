#include "packet.h"
#include "host.h"

/*
Packet::Packet(const Host* source_host, const Host* destination_host){
    source = source_host;
    destination = destination_host;
}
*/

Packet::Packet(const Host* _source, const Host* _destination)
    : source(_source), destination(_destination), destPos(_destination->getPos()), prevPos(nullptr) {}