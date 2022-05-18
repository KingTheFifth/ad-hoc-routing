#ifndef DSRPACKET_H
#define DSRPACKET_H

#include "packet.h"
#include "host/DSRRoute.h"

struct DSRPacket : public Packet {
    DSRPacket() : Packet() {}
    DSRPacket(const Host* _source, const Host* _destination, int _time, int _id = -1);
    
    ~DSRPacket() = default;

    /**
     * Copy 'other' and set other's values to this packet's values
     */
    void copyOther(const Packet& other);

    /** 
     * Make a copy of this packet
     */
    Packet* copy() override;

    //virtual void draw(QGraphicsScene *scene, int x, int y) const;

    enum PacketType {RREQ, RREP, RERR, OTHER};
    unsigned requestID;
    PacketType packetType;
    const Host* errorDestination;
    DSRRoute route;
    int retryCount;
};

#endif
