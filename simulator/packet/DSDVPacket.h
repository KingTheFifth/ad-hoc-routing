#ifndef DSDVPACKET_H
#define DSDVPACKET_H

#include "packet.h"

struct DSDVPacket : public Packet {
    DSDVPacket(const Host* _source, const Host* _destination);

    virtual ~DSDVPacket() = default;
};

#endif