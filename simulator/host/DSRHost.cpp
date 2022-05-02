#include "DSRHost.h"
#include "link.h"

Link* DSRHost::DSR(DSRPacket* packet) {
    for (auto& DSRRoute : routes) {
        if (DSRRoute->hasTarget(packet->destination)) {
            Host* nextHop = DSRRoute->getNextHop(this);
            for (Link* l : neighbours) {
                if (l->getOtherHost(this) == nextHop) {
                    return l;
                }
            }
            // If we reach this line, we have found a broken route
            // Remove this route?
            break;
        }
    }
    
    // if no
        // add packet to DSR specific to-be-sent-buffer
        // send RREQ packet to all neighbours and return nullptr
}

void DSRHost::processPacket(Packet* packet) {
}