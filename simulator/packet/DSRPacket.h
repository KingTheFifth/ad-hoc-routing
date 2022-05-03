#include "packet.h"

struct DSRPacket : public Packet {
    DSRPacket() : Packet() {}
    DSRPacket(const Host* _source, const Host* _destination);
    virtual ~DSRPacket() = default;

    virtual void copyOther(const Packet& other);
    virtual Packet* copy();

    enum PacketType {RREQ, RREP, OTHER};
    PacketType packetType;

};