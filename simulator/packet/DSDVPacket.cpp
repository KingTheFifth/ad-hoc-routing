#include "DSDVPacket.h"
#include "host/host.h"

DSDVPacket::DSDVPacket(const Host* _source, const Host* _destination, int _time, int _id)
            : Packet(_source, _destination, _time, _id), packetType(PacketType::OTHER) {}

void DSDVPacket::copyOther(const Packet& other) {
    const DSDVPacket& DSDVOther = dynamic_cast<const DSDVPacket&>(other);
    Packet::copyOther(DSDVOther);    // Copy base-class data
    packetType = DSDVOther.packetType;
    routingTable = DSDVOther.routingTable;
}

Packet* DSDVPacket::copy() {
    DSDVPacket* copy = new DSDVPacket();
    copy->copyOther(*this);
    return copy;
}
