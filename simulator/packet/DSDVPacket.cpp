#include "DSDVPacket.h"
#include "host/host.h"

DSDVPacket::DSDVPacket(const Host* _source, const Host* _destination)
            : Packet(_source, _destination) {}