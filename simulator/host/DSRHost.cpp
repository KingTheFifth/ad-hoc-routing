#include "DSRHost.h"
#include "link.h"

// will this ever return nullptr?
Link* DSRHost::DSR(DSRPacket* packet) {
    Link* nextHopLink = getCachedRoute(packet->destination);
    if (nextHopLink) {
        return nextHopLink;
    }
    
    // if no
        // add packet to DSR specific to-be-sent-buffer
        // send RREQ packet to all neighbours and return nullptr
    waitingForRouteBuffer.push_back(packet);

    DSRPacket* RREQ = new DSRPacket(this, packet->destination);
    RREQ->packetType = DSRPacket::RREQ;
    RREQ->requestID = requestIDCounter++;
    RREQ->route.addNode(this);

    broadcast(RREQ);
    delete RREQ;
    return nullptr;
}

void DSRHost::processPacket(Packet* packet) {
    DSRPacket* dsrPacket = (DSRPacket*) packet;
    if (dsrPacket->packetType == DSRPacket::RREQ) {
        if (shouldBeDropped(dsrPacket)) {
            delete dsrPacket;
            return;
        }

        recentlySeenRequests.push_back(make_pair(dsrPacket->source, dsrPacket->requestID));

        // Hm, we can either use the destination of a RREQ packet as the "target", or we specify a specific target variable
        // Link* nextHop = getCachedRoute(packet->destination);
        // if (nextHop) {
        //     // Create RREP
        //     //forwardPacket(dsrPacket, l);
        // }
        if (dsrPacket->destination == this) {
            // Create a RREP
            DSRPacket* RREP = new DSRPacket();
            RREP->copyOther(*dsrPacket);
            RREP->packetType = DSRPacket::RREP;
            RREP->destination = dsrPacket->source;
            RREP->source = this;
            RREP->route.addNode(this);

            delete dsrPacket;
            // Send the RREP along the reversed route

            Link* nextHop = getLinkToHost(RREP->route.getNextHop(this, true));
            forwardPacket(RREP, nextHop);
        }
        else {
            dsrPacket->route.addNode(this);
            broadcast(dsrPacket);
            delete dsrPacket;
        }

    }
    else if (dsrPacket->packetType == DSRPacket::RREP) {
        if (dsrPacket->destination == this) {
            for (vector<DSRPacket*>::iterator it = waitingForRouteBuffer.begin(); it != waitingForRouteBuffer.end(); it++) {
                if ((*it)->destination == dsrPacket->source) {
                    (*it)->route = dsrPacket->route;
                    Link* nextHop = getLinkToHost((*it)->route.getNextHop(this, false));
                    forwardPacket((*it), nextHop);
                    waitingForRouteBuffer.erase(it);
                    routes.push_back(new DSRRoute(dsrPacket->route));
                    delete dsrPacket;
                    return; // TODO: Perhaps remove this? -> All packets going to the same destination are sent
                }
            }
            // How did we get here? We got a RREP for a request we have never sent
            delete dsrPacket;
        }
        else {
            DSRRoute* route = new DSRRoute(dsrPacket->route);
            route->trim(this);
            routes.push_back(route);

            Link* nextHop = getLinkToHost(dsrPacket->route.getNextHop(this, true));
            forwardPacket(dsrPacket, nextHop);
        }
    }
    else {
        if (dsrPacket->destination == this) {
            delete dsrPacket;
        }
        else {
            if (!dsrPacket->route.route.empty()) {
                Link* nextHop = getLinkToHost(dsrPacket->route.getNextHop(this, false));
                forwardPacket(dsrPacket, nextHop);
            }
            else {
                // forward normally
                Link* l = DSR(dsrPacket);
                if (l) {
                    forwardPacket(dsrPacket, l);
                }
            }
        }
    }
}

Link* DSRHost::getCachedRoute(const Host* target) {
    for (auto& DSRRoute : routes) {
        if (DSRRoute->hasTarget(target)) {
            const Host* nextHop = DSRRoute->getNextHop(this, false);
            for (Link* l : neighbours) {
                if (l->getOtherHost(this) == nextHop) {
                    return l;
                }
            }
            // TODO: If we reach this line, we have found a broken route
            // Remove this route? Yes. Only remove nodes "reachable" after this one,
            // since they no longer are, but keep prior routes as they were reachable up to this point.
            break;
        }
    }
    // signal cache miss with a nullptr
    return nullptr;
}

bool DSRHost::shouldBeDropped(const DSRPacket* packet) {
    for (auto pair : recentlySeenRequests) {
        if (pair.first == packet->source && pair.second == packet->requestID) {
            return true;
        }
    }

    if(packet->route.hasTarget(this)) return true;
    return false;
}