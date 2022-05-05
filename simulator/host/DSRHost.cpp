#include "DSRHost.h"
#include "link.h"

// will this ever return nullptr?
Link* DSRHost::DSR(DSRPacket* packet) {
    DSRRoute* cachedRoute = getCachedRoute(packet->destination);
    if (cachedRoute) {
        packet->route = *cachedRoute;
        return getLinkToHost(cachedRoute->getNextHop(this, false));
    }
    
    // if no
        // add packet to DSR specific to-be-sent-buffer
        // send RREQ packet to all neighbours and return nullptr
    waitingForRouteBuffer.push_back(packet);

    DSRPacket* RREQ = new DSRPacket(this, packet->destination, time);
    RREQ->packetType = DSRPacket::RREQ;
    RREQ->requestID = requestIDCounter++;
    RREQ->route.addNode(this);

    broadcast(RREQ);
    statistics->packetsSent += neighbours.size();
    statistics->routingPacketsSent += neighbours.size();
    delete RREQ;
    return nullptr;
}

void DSRHost::processPacket(Packet* packet) {
    DSRPacket* dsrPacket = (DSRPacket*) packet;
    
    if (dsrPacket->packetType == DSRPacket::RREQ) { // RREQ packets
        if (shouldBeDropped(dsrPacket)) {
            delete dsrPacket;
            return;
        }

        recentlySeenRequests.push_back(make_pair(dsrPacket->source, dsrPacket->requestID));

        // Link* nextHop = getCachedNextHop(packet->destination);
        // if (nextHop) {
        //     // Create RREP
        //     //forwardPacket(dsrPacket, l);
        // }
        if (dsrPacket->destination == this) { // Arrived at destination
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
            statistics->packetsSent++;
            statistics->routingPacketsSent++;
        }
        else { // Not yet arrived
            DSRRoute* cachedRoute = getCachedRoute(dsrPacket->destination);
            if (cachedRoute) { // If this host has a cached route to the destination
                DSRPacket* RREP = new DSRPacket();
                RREP->copyOther(*dsrPacket);
                RREP->packetType = DSRPacket::RREP;
                RREP->destination = dsrPacket->source;
                RREP->source = dsrPacket->destination;
                
                RREP->route.addRoute(cachedRoute);

                Link* nextHop = getLinkToHost(RREP->route.getNextHop(this, true));
                forwardPacket(RREP, nextHop);

                statistics->packetsSent++;
                statistics->routingPacketsSent++;
            }
            else { // No cached route to destination from this host
                dsrPacket->route.addNode(this);
                if (!getCachedNextHop(dsrPacket->source)) { // If we have not cached the route from this host to the source
                    DSRRoute* reversedRouteCopy = new DSRRoute(dsrPacket->route, true);
                    routes.push_back(reversedRouteCopy);
                }
                statistics->packetsSent += neighbours.size();
                statistics->routingPacketsSent += neighbours.size();
                broadcast(dsrPacket);
            }

            delete dsrPacket;
        }
    }

    else if (dsrPacket->packetType == DSRPacket::RREP) { // RREP packets
        if (dsrPacket->destination == this) { // Arrived at destination
            for (vector<DSRPacket*>::iterator it = waitingForRouteBuffer.begin(); it != waitingForRouteBuffer.end(); it++) {
                if ((*it)->destination == dsrPacket->source) { // If 'it' is the packet we have been waiting to send
                    (*it)->route = dsrPacket->route;
                    Link* nextHop = getLinkToHost((*it)->route.getNextHop(this, false));
                    forwardPacket((*it), nextHop);
                    waitingForRouteBuffer.erase(it);
                    routes.push_back(new DSRRoute(dsrPacket->route));

                    // statistics->packetsSent++;
                    // statistics->dataPacketsSent++;
                    delete dsrPacket;
                    return; // TODO: Perhaps remove this? -> All packets going to the same destination are sent
                }
            }
            // We got a RREP for a request we have already processed
            delete dsrPacket;
        }
        else { // Not yet at destination
            DSRRoute* route = new DSRRoute(dsrPacket->route);
            route->trimFront(this);
            routes.push_back(route);

            Link* nextHop = getLinkToHost(dsrPacket->route.getNextHop(this, true));
            forwardPacket(dsrPacket, nextHop);
        }
    }

    else { // Other packets
        if (dsrPacket->destination == this) { // Arrived at destination
            int delay = time - dsrPacket->timeSent;
            unsigned prevDelaySum = statistics->avgDelay * statistics->dataPacketsArrived;
            double prevThroughputSum = statistics->avgThroughput * statistics->dataPacketsArrived;
            statistics->dataPacketsArrived++;
            statistics->avgDelay = (double) (prevDelaySum + delay) / (double) statistics->dataPacketsArrived;
            statistics->avgThroughput = (prevThroughputSum + 1.0 / (double) delay) / (double) statistics->dataPacketsArrived;
            delete dsrPacket;
        }
        else { // Not yet at destination
            if (!dsrPacket->route.isEmpty()) { // If the packet already has a route to follow
                Link* nextHop = getLinkToHost(dsrPacket->route.getNextHop(this, false));
                forwardPacket(dsrPacket, nextHop);
            }
            else { // No route to follow, send the packet normally
                Link* l = DSR(dsrPacket);
                if (l) {                   
                    // statistics->packetsSent++;
                    // statistics->dataPacketsSent++;
                    forwardPacket(dsrPacket, l);
                }
            }
        }
    }
}

Link* DSRHost::getCachedNextHop(const Host* target) {
    DSRRoute* cachedRoute = getCachedRoute(target);
    if (!cachedRoute) return nullptr;

    const Host* cachedNextHop = cachedRoute->getNextHop(this, false);
    for (Link* l : neighbours) {
        if (l->getOtherHost(this) == cachedNextHop) {
            return l;
        }
    }
 
    // TODO: If we reach this line, we have found a broken route
    // Remove this route? Yes. Only remove nodes "reachable" after this one,
    // since they no longer are, but keep prior routes as they were reachable up to this point.
    return nullptr;
}

DSRRoute* DSRHost::getCachedRoute(const Host* target) {
    for (auto& DSRRoute : routes) {
        if (DSRRoute->hasTarget(target)) {
            return DSRRoute;
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
