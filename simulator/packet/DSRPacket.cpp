#include "DSRPacket.h"
#include "host/host.h"

DSRPacket::DSRPacket(const Host* _source, const Host* _destination, int _time)
            : Packet(_source, _destination, _time), packetType(PacketType::OTHER) {}

void DSRPacket::copyOther(const Packet& other) {
    const DSRPacket& DSROther = dynamic_cast<const DSRPacket&>(other);
    Packet::copyOther(DSROther);    // Copy base-class data
    packetType = DSROther.packetType;
    route = DSROther.route;
    requestID = DSROther.requestID;
}

Packet* DSRPacket::copy() {
    DSRPacket* copy = new DSRPacket();
    copy->copyOther(*this);
    return copy;
}