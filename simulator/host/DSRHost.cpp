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
    // Host::~Host();
}

void DSRHost::tick(int currTime) {
    int timeDelta = currTime - time;
    Host::tick(currTime);
    
    if (timeDelta > 0) {
        for (vector<pair<DSRPacket*, int>>::iterator it = waitingForRouteBuffer.begin(); it != waitingForRouteBuffer.end(); it++) {
            it->second -= timeDelta;

            if (it->second <= 0) {
                cout << "Packet from " << it->first->source << " to " << it->first->destination << "timed out" << endl;
                it->first->retryCount++;
                if (receivingBuffer.size()*PACKET_SIZE >= HOST_BUFFER_SIZE) {
                    it->second = DSR_TIMEOUT / 2;
                } else {
                    receivePacket(it->first);
                    it = waitingForRouteBuffer.erase(it) - 1;
                }
            }

            else if (it->first->retryCount >= MAX_RETRY_COUNT) {
                cout << "Packet from " << it->first->source->id << " to " << it->first->destination->id << "reached retry count " << it->first->retryCount << endl;
                DSRPacket* p = it->first;
                it = waitingForRouteBuffer.erase(it) - 1;
                dropReceivedPacket(p);
            }
        }
    }
}

void DSRHost::DSR(DSRPacket* packet) {
    DSRRoute* cachedRoute = getCachedRoute(packet->destination);
    if (cachedRoute) {
        packet->route.copyOther(*cachedRoute, false);
        Link* nextHop = getLinkToHost(cachedRoute->getNextHop(this, false));
        if (nextHop) {
            forwardPacket(packet, nextHop);
            return;
        }
        // We have found a broken cached route
    }

    waitingForRouteBuffer.push_back(make_pair(packet, DSR_TIMEOUT));

    DSRPacket* RREQ = new DSRPacket(this, packet->destination, time);
    RREQ->packetType = DSRPacket::RREQ;
    RREQ->requestID = requestIDCounter++;
    RREQ->route.addNode(this);

    broadcast(RREQ);
    statistics->addRoutingPackets(neighbours.size());
    delete RREQ;
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
            statistics->addRoutingPackets(1);
        }
        else { // Not yet arrived
            DSRRoute* cachedRoute = getCachedRoute(dsrPacket->destination);
            if (cachedRoute) { // If this host has a cached route to the destination
                if (cachedRoute->hasTarget(dsrPacket->source)) {
                    cout << "RREQ for packet to host " << dsrPacket->destination->id << ", cached route: " << cachedRoute << endl;
                    // If we want to route through the source, but the source does not know how to reach 
                    // the destination then we have reached some sort of invalid route
                    cachedRoute->trimBack(dsrPacket->source);
                    dropReceivedPacket(dsrPacket);
                    return;
                }

                DSRPacket* RREP = new DSRPacket();
                RREP->copyOther(*dsrPacket);
                RREP->packetType = DSRPacket::RREP;
                RREP->destination = dsrPacket->source;
                RREP->source = dsrPacket->destination;
                
                RREP->route.addRoute(cachedRoute);

                Link* nextHop = getLinkToHost(RREP->route.getNextHop(this, true));
                if (nextHop) {
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
            for (vector<pair<DSRPacket*, int>>::iterator it = waitingForRouteBuffer.begin(); it != waitingForRouteBuffer.end(); it++) {
                if (it->first->destination == dsrPacket->source) { // If 'it' is the packet we have been waiting to send
                    it->first->route.copyOther(dsrPacket->route, false);
                    Link* nextHop = getLinkToHost(it->first->route.getNextHop(this, false));
                    forwardPacket(it->first, nextHop);
                    waitingForRouteBuffer.erase(it);
                    routes.push_back(new DSRRoute(dsrPacket->route));

                    delete dsrPacket;
                    return; // TODO: Perhaps remove this? -> All packets going to the same destination are sent
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
        if (dsrPacket->destination == this) {
            // Delete cached route to dsrPacket->errorDestination (trim from source of packet to errorDestination)
            handleRERR(dsrPacket);

            dsrPacket->packetType = DSRPacket::OTHER;
            dsrPacket->source = this;
            dsrPacket->destination = dsrPacket->errorDestination;
            dsrPacket->route.empty();
            dsrPacket->retryCount++;
            receivePacket(dsrPacket);
        }
        else {
            handleRERR(dsrPacket);
            Link* nextHop = getLinkToHost(dsrPacket->route.getNextHop(this, true));
            if (nextHop) {
                forwardPacket(dsrPacket, nextHop);
                return;
            }
            dropReceivedPacket(dsrPacket);
            return;
        }
    }

    else { // Other packets
        if (dsrPacket->destination == this) { // Arrived at destination
            int delay = time - dsrPacket->timeSent;
            statistics->addPacketArrival(delay);
            delete dsrPacket;
            return;
        }
        else { // Not yet at destination
            if (!dsrPacket->route.isEmpty()) { // If the packet already has a route to follow
                // TODO: If link does not exist, send RERR back to source
                Link* nextHop = getLinkToHost(dsrPacket->route.getNextHop(this, false));
                if (!nextHop) sendRERR(dsrPacket);
                else forwardPacket(dsrPacket, nextHop);
            }
            else { // No route to follow, send the packet normally
                DSR(dsrPacket);
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


void DSRHost::dropReceivedPacket(Packet* packet) {
    countPacketDrop(packet);
    delete packet;
}

void DSRHost::countPacketDrop(Packet* packet) {
    DSRPacket* dsrPacket = dynamic_cast<DSRPacket*>(packet);
    if (dsrPacket->packetType == DSRPacket::PacketType::OTHER || dsrPacket->packetType == DSRPacket::PacketType::RERR) {
        statistics->dropDataPacket();
    }
}

void DSRHost::deleteRoutes(Host* destination) {
    return;
}

void DSRHost::sendRERR(DSRPacket* packet) {
    
    Link* nextHopLink = getLinkToHost(packet->route.getNextHop(this, true));
    
    if (nextHopLink) {
        cout << "Sending RERR to " << packet->source->id << " for " << packet->destination->id << ", from " << id << endl;
        packet->errorDestination = packet->destination;
        packet->destination = packet->source;
        packet->source = this;
        packet->packetType = DSRPacket::RERR;
        forwardPacket(packet, nextHopLink);
    }
    else { // The route I was supposed to follow had a broken link, so I tried to reverse, but the link I came from is gone (basically fml) - packet, May 2022
        // TODO: this should be dropped as a data packet
        dropReceivedPacket(packet);
    }
}

void DSRHost::handleRERR(DSRPacket* packet) {
    const Host* brokenLink = packet->route.getNextHop(packet->source, false);
    // for (vector<DSRRoute*>::iterator it = routes.begin(); it != routes.end(); it++) {
    //     DSRRoute* route = *it;
    //     if (route->hasTarget(brokenLink)) {
    //         route->trimBack(packet->source);

    //         if (route->size() == 1) {
    //             it = routes.erase(it) - 1;
    //             delete route;
    //         }
    //     }
    // }

    vector<DSRRoute*>::iterator it = routes.begin();
    while(it != routes.end()) {
        DSRRoute* route = *it;
        if (route->hasTarget(brokenLink)) {
            route->trimBack(packet->source);

            if (route->size() == 1) {
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
    for (auto& packetInBuffer : waitingForRouteBuffer) {
        dropReceivedPacket(packetInBuffer.first);
    }
}
