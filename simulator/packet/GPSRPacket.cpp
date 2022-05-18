#include "GPSRPacket.h"
#include "host/host.h"

GPSRPacket::GPSRPacket(const Host* _source, const Host* _destination, int _time, int _id)
            : Packet(_source, _destination, _time, _id), destPos(_destination->getPos()), prevPos(nullptr), failurePos(nullptr) {}

/*Packet* copy() {
    return nullptr;
}*/
