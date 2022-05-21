#include "DSRHost.h"
#include "link.h"

DSRHost::~DSRHost() {
    for (auto& route : routes) {
        delete route;
    }
    for (auto& packetInBuffer : waitingForRouteBuffer) {
        delete packetInBuffer.first;
    }
    while (!transmitBuffer.empty()) {
        Packet* p = transmitBuffer.front().first;
        transmitBuffer.pop();
        dropReceivedPacket(p);
    }

    while (!receivingBuffer.empty()) {
        Packet* p = receivingBuffer.front();
        receivingBuffer.pop();
        dropReceivedPacket(p);
    }
}

void DSRHost::tick(int currTime) {
    int timeDelta = currTime - time;
    Host::tick(currTime); // Tick normally using the base class tick function
    
    if (timeDelta > 0) {
        for (vector<pair<DSRPacket*, int>>::iterator it = waitingForRouteBuffer.begin(); it != waitingForRouteBuffer.end(); it++) { // For all packets in the waiting buffer
            it->second -= timeDelta;

            if (it->second <= 0) { // If it has timed out
                it->first->retryCount++; // Increment its retry count
                if (receivingBuffer.size()*PACKET_SIZE >= HOST_BUFFER_SIZE) { // If we still have space for the packet
                    it->second = DSR_TIMEOUT / 2; // Re-set its timeout
                } else {
                    receivePacket(it->first); // Try to receive the packet, this will fail and the packet will be dropped
                    it = waitingForRouteBuffer.erase(it) - 1;
                }
            }

            else if (it->first->retryCount >= MAX_RETRY_COUNT) { // If the packet has reached its maximum retry count
                // Remove it from the buffer and drop it
                DSRPacket* p = it->first;
                it = waitingForRouteBuffer.erase(it) - 1;
                dropReceivedPacket(p);
            }
        }
    }
}

void DSRHost::DSR(DSRPacket* packet) {
    DSRRoute* cachedRoute = getCachedRoute(packet->destination);
    if (cachedRoute) { // If this host already has a cached rout to the destination
        packet->route.copyOther(*cachedRoute, false); // Set its intended route
        Link* nextHop = getLinkToHost(cachedRoute->getNextHop(this, false)); // Get the link it should be forwarded on
        if (nextHop) { // If the link is not broken
            forwardPacket(packet, nextHop);
            return;
        }
        // We have found a broken cached route, proceed as if the route was not already cached
    }

    waitingForRouteBuffer.push_back(make_pair(packet, DSR_TIMEOUT)); // Put the packet in the waiting buffer

    // Create a Route Request packet to find a route to the packets intended destination
    DSRPacket* RREQ = new DSRPacket(this, packet->destination, time);
    RREQ->packetType = DSRPacket::RREQ;
    RREQ->requestID = requestIDCounter++;
    RREQ->route.addNode(this);

    broadcast(RREQ); // Broadcast the packet to all of this host's neighbours
    statistics->addRoutingPackets(neighbours.size());
    delete RREQ;
}

void DSRHost::processPacket(Packet* packet) {
    DSRPacket* dsrPacket = (DSRPacket*) packet;
    
    if (dsrPacket->packetType == DSRPacket::RREQ) { // RREQ packets
        if (shouldBeDropped(dsrPacket)) { // Drop the packet if we should do so
            delete dsrPacket;
            return;
        }

        // Add the packet as recently seen
        recentlySeenRequests.push_back(make_pair(dsrPacket->source, dsrPacket->requestID));

        if (dsrPacket->destination == this) { // Arrived at destination
            // Create a Route Reply packet (RREP)
            DSRPacket* RREP = new DSRPacket();
            RREP->copyOther(*dsrPacket);
            RREP->packetType = DSRPacket::RREP;
            RREP->destination = dsrPacket->source;
            RREP->source = this;
            RREP->route.addNode(this);

            delete dsrPacket;

            // Send the RREP along the reversed route
            Link* nextHop = getLinkToHost(RREP->route.getNextHop(this, true));
            if(nextHop) { // If the link has not been broken
                forwardPacket(RREP, nextHop);
                statistics->addRoutingPackets(1);
            } else {
                dropReceivedPacket(RREP);
            }
        }
        else { // Not yet arrived
            DSRRoute* cachedRoute = getCachedRoute(dsrPacket->destination);
            if (cachedRoute) { // If this host has a cached route to the destination
                if (cachedRoute->hasTarget(dsrPacket->source)) {
                    // If we want to route through the source, but the source does not know how to reach 
                    // the destination, then we have reached some sort of invalid route
                    cachedRoute->trimBack(dsrPacket->source);
                    dropReceivedPacket(dsrPacket);
                    return;
                }

                // Create a Route Reply packet (RREP) with the concatenated route the packet
                // took to this node and the route from this node to the destination
                DSRPacket* RREP = new DSRPacket();
                RREP->copyOther(*dsrPacket);
                RREP->packetType = DSRPacket::RREP;
                RREP->destination = dsrPacket->source;
                RREP->source = dsrPacket->destination;
                RREP->route.addRoute(cachedRoute);

                Link* nextHop = getLinkToHost(RREP->route.getNextHop(this, true));
                if (nextHop) { // If the link is not broken
                    forwardPacket(RREP, nextHop);
                    statistics->addRoutingPackets(1);
                }
                
            }
            else { // No cached route to destination from this host
                dsrPacket->route.addNode(this);
                if (!getCachedNextHop(dsrPacket->source)) { // If we have not cached the route from this host to the source
                    DSRRoute* reversedRouteCopy = new DSRRoute(dsrPacket->route, true);
                    routes.push_back(reversedRouteCopy);
                }
                statistics->addRoutingPackets(neighbours.size());
                broadcast(dsrPacket);
            }

            delete dsrPacket;
            return;
        }
    }

    else if (dsrPacket->packetType == DSRPacket::RREP) { // RREP packets
        if (dsrPacket->destination == this) { // Arrived at destination
            for (vector<pair<DSRPacket*, int>>::iterator it = waitingForRouteBuffer.begin(); it != waitingForRouteBuffer.end(); it++) { // For all packets in the waiting buffer
                if (it->first->destination == dsrPacket->source) { // If 'it' is the packet we have been waiting to send
                    // Attempt to forward the packet
                    it->first->route.copyOther(dsrPacket->route, false);
                    Link* nextHop = getLinkToHost(it->first->route.getNextHop(this, false));
                    if(nextHop) { // If link is not broken
                        forwardPacket(it->first, nextHop);
                        waitingForRouteBuffer.erase(it);
                        routes.push_back(new DSRRoute(dsrPacket->route));
                        delete dsrPacket;
                    } else {
                        dropReceivedPacket(dsrPacket);
                    }
                    return;
                }
            }
            // We got a RREP for a request we have already processed
            delete dsrPacket;
            return;
        }
        else { // Not yet at destination
            // If the reply contains a route that this host does not know, cache it
            if(!getCachedNextHop(dsrPacket->destination)) {
                DSRRoute* route = new DSRRoute(dsrPacket->route);
                route->trimFront(this);
                routes.push_back(route);
            }

            // Attempt to forward the packet according to its next hop
            Link* nextHop = getLinkToHost(dsrPacket->route.getNextHop(this, true));
            if (nextHop) {
                forwardPacket(dsrPacket, nextHop);
                return;
            }
            else {
                delete dsrPacket;
                return;
            }
        }
    }

    else if (dsrPacket->packetType == DSRPacket::RERR) { // RERR packets
        if (dsrPacket->destination == this) { // If this was the host that sent the data packet originally
            // Delete cached route to dsrPacket->errorDestination (trim from source of packet to errorDestination)
            handleRERR(dsrPacket);

            // Try retransmitting the packet as a data packet once again
            dsrPacket->packetType = DSRPacket::OTHER;
            dsrPacket->source = this;
            dsrPacket->destination = dsrPacket->errorDestination;
            dsrPacket->route.empty();
            dsrPacket->retryCount++;
            receivePacket(dsrPacket);
        }
        else {
            handleRERR(dsrPacket); // Trim routes

            // Attempt to forward the packet according to its next hop
            Link* nextHop = getLinkToHost(dsrPacket->route.getNextHop(this, true));
            if (nextHop) {
                forwardPacket(dsrPacket, nextHop);
                return;
            }
            dropReceivedPacket(dsrPacket);
            return;
        }
    }

    else { // Other packets (data packets!)
        if (dsrPacket->destination == this) { // If arrived at destination
            // Count the packet
            int delay = time - dsrPacket->timeSent;
            statistics->addPacketArrival(delay);
            delete dsrPacket;
            return;
        }
        else { // Not yet at destination
            if (!dsrPacket->route.isEmpty()) { // If the packet already has a route to follow
                Link* nextHop = getLinkToHost(dsrPacket->route.getNextHop(this, false));
                if (!nextHop) sendRERR(dsrPacket); // If the link is broken, send a RERR packet
                else forwardPacket(dsrPacket, nextHop);
            }
            else { // No route to follow, send the packet normally
                DSR(dsrPacket);
            }
        }
    }
}

Link* DSRHost::getCachedNextHop(const Host* target) {
    DSRRoute* cachedRoute = getCachedRoute(target); // Check if we have a cached route to 'target'
    if (!cachedRoute) return nullptr; // If we do not, return nullptr

    const Host* cachedNextHop = cachedRoute->getNextHop(this, false);
    for (Link* l : neighbours) { // Find the next link to forward to
        if (l->getOtherHost(this) == cachedNextHop) {
            return l;
        }
    }
    return nullptr; // Return nullptr if no link was found where it should have been (cached link has been broken)
}

DSRRoute* DSRHost::getCachedRoute(const Host* target) {
    for (auto& DSRRoute : routes) { // For all cached routes
        if (DSRRoute->hasTarget(target)) { // If we find a route with 'target'
            return DSRRoute;
        }
    }
    return nullptr; // Signal cache miss with a nullptr
}

bool DSRHost::shouldBeDropped(const DSRPacket* packet) {
    for (auto pair : recentlySeenRequests) { // For all recently seen requests
        if (pair.first == packet->source && pair.second == packet->requestID) { // If the packet has the same source and requestID as a recently seen packet
            return true;
        }
    }

    if(packet->route.hasTarget(this)) return true; // If this host is the intended target for the packet
    return false;
}


void DSRHost::dropReceivedPacket(Packet* packet) {
    countPacketDrop(packet);
    delete packet;
}

void DSRHost::countPacketDrop(Packet* packet) {
    DSRPacket* dsrPacket = dynamic_cast<DSRPacket*>(packet);

    // Only count the packet toward the statistics if it is a data packet
    if (dsrPacket->packetType == DSRPacket::PacketType::OTHER || dsrPacket->packetType == DSRPacket::PacketType::RERR) {
        statistics->dropDataPacket();
    }
}

void DSRHost::deleteRoutes(Host* destination) {
    return; // Unused in DSR
}

void DSRHost::sendRERR(DSRPacket* packet) {
    Link* nextHopLink = getLinkToHost(packet->route.getNextHop(this, true));
    
    if (nextHopLink) { // If the link we came from is not broken
        // Change this data packet into a Route Error packet (RERR)
        packet->errorDestination = packet->destination;
        packet->destination = packet->source;
        packet->source = this;
        packet->packetType = DSRPacket::RERR;
        forwardPacket(packet, nextHopLink);
    }
    else { // The route I was supposed to follow had a broken link, so I tried to send a RERR and reverse, but the link I came from is gone (basically fml) - packet, May 2022
        dropReceivedPacket(packet);
    }
}

void DSRHost::handleRERR(DSRPacket* packet) {
    const Host* brokenLink = packet->route.getNextHop(packet->source, false); // Find the broken link

    vector<DSRRoute*>::iterator it = routes.begin();
    while(it != routes.end()) { // For all routes
        DSRRoute* route = *it;
        if (route->hasTarget(brokenLink)) { // If it contains the broken link
            route->trimBack(packet->source); // Remove all destinations in the route after the broken link

            if (route->size() == 1) { // If the route only contains this host (the broken link was to a direct neighbour)
                // Delete the route entirely
                delete route;
                it = routes.erase(it);
            }
            else {
                it++;
            }
        }
        else {
            it++;
        }
    }
}

void DSRHost::die() {
    Host::die();
    for (auto& packetInBuffer : waitingForRouteBuffer) { // Empty all packets in the waiting buffer
        dropReceivedPacket(packetInBuffer.first);
    }
}
