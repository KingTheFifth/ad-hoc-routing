#ifndef DSRPACKET_H
#define DSRPACKET_H

#include "packet.h"
#include "host/DSRRoute.h"

struct DSRPacket : public Packet {
    DSRPacket() : Packet() {}
    DSRPacket(const Host* _source, const Host* _destination);
    virtual ~DSRPacket() = default;

    void copyOther(const Packet& other);
    Packet* copy() override;

    enum PacketType {RREQ, RREP, OTHER};
    unsigned requestID;
    PacketType packetType;
    DSRRoute route;
};

#endif