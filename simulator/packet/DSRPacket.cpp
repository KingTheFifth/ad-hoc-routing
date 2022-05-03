#include "DSRPacket.h"
#include "host/host.h"

DSRPacket::DSRPacket(const Host* _source, const Host* _destination)
            : Packet(_source, _destination) {}

void DSRPacket::copyOther(const Packet& other) {
    const DSRPacket& DSROther = dynamic_cast<const DSRPacket&>(other);
    Packet::copyOther(DSROther);    // Copy base-class data
    packetType = DSROther.packetType;
}

Packet* DSRPacket::copy() {
    DSRPacket* copy = new DSRPacket();
    copy->copyOther(*this);
    return copy;
}