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

    /**
     * Copy 'other' and set other's values to this packet's values
     */
    void copyOther(const Packet& other);

    /** 
     * Make a copy of this packet
     */
    Packet* copy() override;
};

#endif