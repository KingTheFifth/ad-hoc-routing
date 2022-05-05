#ifndef DSDVPACKET_H
#define DSDVPACKET_H

#include "packet.h"
#include "host/routingTable.h"

struct DSDVPacket : public Packet {
    DSDVPacket() = default;

    DSDVPacket(const Host* _source, const Host* _destination, int _time);

    virtual ~DSDVPacket() = default;
    enum PacketType {BROADCAST, OTHER};
    PacketType packetType;
    RoutingTable* routingTable;

    void copyOther(const Packet& other);
    Packet* copy() override;
};

#endif