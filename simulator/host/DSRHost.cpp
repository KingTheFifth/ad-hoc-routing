#include "DSRHost.h"
#include "link.h"

// will this ever return nullptr?
Link* DSRHost::DSR(DSRPacket* packet) {
    Link* nextHopLink = getCachedRoute(packet->destination);
    if (nextHopLink != nullptr) return nextHopLink;
    
    // if no
        // add packet to DSR specific to-be-sent-buffer
        // send RREQ packet to all neighbours and return nullptr
    waitingForRouteBuffer.push_back(packet);
    DSRPacket* RREQ = new DSRPacket();
    broadcast(RREQ);
    return nullptr;
}

void DSRHost::processPacket(Packet* packet) {
    DSRPacket* dsrPacket = (DSRPacket*) packet;
    if (dsrPacket->packetType == DSRPacket::RREQ) {
        // Hm, we can either use the destination of a RREQ packet as the "target", or we specify a specific target variable
        // getCachedRoute(packet->destination);
    }
    else if (dsrPacket->packetType == DSRPacket::RREP) {

    }
    else {
        if (dsrPacket->destination == this)
            cout << "very nice dsr data packet has arrived :)" << '\n';
        else {
            // forward normally
            Link* l = DSR(dsrPacket);
            if (l != nullptr) {
                forwardPacket(dsrPacket, l);
            }
        }
    }
    // if RREQ
        // if we have route to target cached send RREP to source(?)
        // else
            // if already gone through this host (transaction ID && source host match), drop
            // else broadcast to all neighbours with itself added to route

    // if RREP
        // if to this host
            // Find packet in waitingForRouteBuffer corresponding to the RREP
            // Set the route in the packet + add to the toBeSentBuffer
            
        // else forward using route indicated in packet
    // else
        // if arrived
            // :)
        // else forward normally
}

Link* DSRHost::getCachedRoute(const Host* target) {
    for (auto& DSRRoute : routes) {
        if (DSRRoute->hasTarget(target)) {
            Host* nextHop = DSRRoute->getNextHop(this);
            for (Link* l : neighbours) {
                if (l->getOtherHost(this) == nextHop) {
                    return l;
                }
            }
            // If we reach this line, we have found a broken route
            // Remove this route? Yes.
            break;
        }
    }
    // signal cache miss with a nullptr
    return nullptr;
}