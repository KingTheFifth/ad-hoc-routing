#include "GPSRPacket.h"
#include "host/host.h"

GPSRPacket::GPSRPacket(const Host* _source, const Host* _destination)
            : Packet(_source, _destination), destPos(_destination->getPos()), prevPos(nullptr), failurePos(nullptr) {}

/*Packet* copy() {
    return nullptr;
}*/