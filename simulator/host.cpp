#include "constants.h"
#include "link.h"
#include "host.h"
#include "packet.h"
#include <cmath>

void Host::discoverNeighbours(vector<Host*>* hosts) {
    for (auto& host : *hosts) {
        double distance = location->distanceTo(host->location);
        if (host != this && distance < radius) {
            addNeighbour(host);
        }
    }
}

void Host::addNeighbour(Host* host) {
    for (auto& link : neighbours) {
        if (link->getOtherHost(this) == host) return;
    }
    
    Link* newLink = new Link(this, host, time);
    neighbours.push_back(newLink);
    host->neighbours.push_back(newLink);
}

Point* Host::getPos() const {
    return location;
}

void Host::draw(QGraphicsScene *scene) const {
    location->draw(scene);
    for (auto& neighbour : neighbours) {
        neighbour->draw(scene);
    }
    if (perimDrawCountdown > 0) {
        for(auto& perimeter : perimeterLinks) {
            perimeter->drawAsPerimeter(scene);
        }
    }
}

void Host::tick(int currTime) {
    time = currTime;

    if (perimDrawCountdown > 0) perimDrawCountdown--;

    for (auto& link : neighbours) {
        link->tick(currTime);
    }
    /*
    if (buffer not empty && processing delay is over)
        do routing
    */
    if (processingCountdown == 0 && !buffer.empty()) {
        if (processingPacket != nullptr) {
            Link* l = GPSR(processingPacket);
            if (l != nullptr)
                forwardPacket->(processingPacket, l)
            else
                delete processingPacket;
        }

        Packet* packet = buffer.front();
        buffer.pop();

        // assuming GPSR
        if (location->distanceTo(packet->destPos) < CLOSE_THRESHOLD) {
            delete packet;
        }
        else {
            // process and send packet later
            processingPacket = packet;
            processingCountdown = HOST_PROCESSING_DELAY;
        }

        // // obv have this globally somehow
        // enum Protocol {DSDV, DSR, GPSR};
        // Protocol protocol = GPSR;

        // switch (protocol) {
        //     case DSDV: // do DSDV receival
        //     case DSR: // do DSR receival
        //     case GPSR: // do GPSR receival
        // }
        
    }
    if (processingCountdown > 0)
        processingCountdown--; // use timedelta instead!
    
    if (mobilityTarget != nullptr) {
        if (location->distanceTo(mobilityTarget) < CLOSE_THRESHOLD) {
            mobilityTarget = nullptr;
        }
        else {
            // move towards mobilityTarget
            double step = HOST_MOVEMENT_STEP;
            double angle = location->angleTo(mobilityTarget);
            double dy = sin(angle) * step;
            double dx = cos(angle) * step;

            location->y += dy;
            location->x += dx;
        }
    }
    
}

void Host::forwardPacket(Packet *packet, Link *link) {
    perimDrawCountdown = 20;
    packet->prevPos = location;
    packet->nextHop = link->getOtherHost(this);
    link->forwardPacket(packet);
}

void Host::receivePacket(Packet* packet) {
    if (buffer.size()*PACKET_SIZE >= HOST_BUFFER_SIZE) {
        // drop packet
        delete packet; // ?
    } else {
        buffer.push(packet);
    }
}

void Host::moveTo(Point* target) {
    mobilityTarget = target;
}

Link* Host::DSR(Packet* packet) {
    for (auto& DSRRoute : routes) {
        // something
    }
    // do we have the destination in the cache?
    // if yes, send it according to cache
    // if no
        // add packet to DSR specific to-be-sent-buffer
        // send RREQ packet to all neighbours and return nullptr
}

Link* Host::DSDV(Packet* packet) {
    // Do DSDV
}

Link* Host::GPSR(Packet* packet) {
    Point* previous = packet->prevPos;
    const Point* destination = packet->destPos;

    // Swap to Greedy mode if we are closer to the destination now than from the point of Greedy failure
    if (packet->mode == Packet::Perimeter &&
        location->distanceTo(destination) < packet->failurePos->distanceTo(destination)) {
        packet->mode = Packet::Greedy;
    }

    // Find the neighbour geographically closest to the destination
    Point* closestPos = location;
    Link* nextHopLink = nullptr;
    if (packet->mode == Packet::Greedy) {
        for (Link* link : neighbours) {
            Point* pos = link->getOtherHost(this)->getPos();
            if (pos->distanceTo(destination) < closestPos->distanceTo(destination)) {
                closestPos = pos;
                nextHopLink = link;
            }
        }
    }

    // If no geographically closer neighbour is found, switch to Perimeter routing
    if (closestPos->distanceTo(location) == 0 && packet->mode == Packet::Greedy) {
        packet->mode = Packet::Perimeter;
        packet->failurePos = location;

        vector<Link*> perimeter;
        getPerimeterLinks(&perimeter);
        getPerimeterLinks(&perimeterLinks);
        nextHopLink = getRHREdge(destination, &perimeter);
        packet->firstEdgeInPerim = nextHopLink;
    }
    else if (packet->mode == Packet::Perimeter) {
        vector<Link*> perimeter;
        getPerimeterLinks(&perimeter);
        getPerimeterLinks(&perimeterLinks);
        Link* RHREdge = getRHREdge(previous, &perimeter);
        
        // Drop packet if host is isolated
        if (!RHREdge) return nullptr;
        Point* Lf = getCrossing(location, RHREdge->getOtherHost(this)->getPos(), packet->failurePos, destination);

        if (
            Lf &&
            (!packet->destLineIntersect ||
            Lf->distanceTo(destination) < packet->destLineIntersect->distanceTo(destination))
        ) {
            packet->destLineIntersect = Lf;
            Point* newRHRReference = RHREdge->getOtherHost(this)->getPos();
            RHREdge = getRHREdge(newRHRReference, &perimeter);
            packet->firstEdgeInPerim = RHREdge;
        }

        // Drop packet if whole perimeter is traversed (i.e. destination could not be reached)
        if (RHREdge == packet->firstEdgeInPerim) return nullptr;
        nextHopLink = RHREdge; 
    }
    return nextHopLink;
}

// can return null if this host is isolated
Link* Host::getRHREdge(const Point* referencePoint, vector<Link*>* perimeterLinks) const {
    double referenceAngle = location->angleTo(referencePoint);
    double smallestAngle = std::numeric_limits<double>::infinity();
    Link* RHREdge = nullptr;
    for (Link* link : *perimeterLinks) {
        Point* perimNeighbourPos = link->getOtherHost(this)->getPos();
        double perimNeighbourAngle = location->angleTo(perimNeighbourPos);
        if (perimNeighbourAngle <= referenceAngle) perimNeighbourAngle += 2*PI;
        
        double angleDiff = perimNeighbourAngle - referenceAngle;
        if (angleDiff < smallestAngle) {
            smallestAngle = angleDiff;
            RHREdge = link;
        }
    }
    return RHREdge;
}

void Host::getPerimeterLinks(vector<Link*>* result) {
    *result = neighbours;
    
    // Remove crossing links as per RNG
    vector<Link*>::iterator l1 = neighbours.begin(); 
    while (l1 != neighbours.end()) {
        vector<Link*>::iterator l2 = neighbours.begin();
        while (l2 != neighbours.end()) {
            if (l1 == l2) {
                l2++;
                continue;
            }
            
            Host* n1 = (*l1)->getOtherHost(this);
            Host* n2 = (*l2)->getOtherHost(this);
            double distThisToN1 = location->distanceTo(n1->getPos());
            double distThisToN2 = location->distanceTo(n2->getPos());
            double distN1ToN2 = n1->getPos()->distanceTo(n2->getPos());
            
            if (distThisToN1 > max(distThisToN2, distN1ToN2)) {
                vector<Link*>::iterator l3 = result->begin();
                while (l3 != result->end()) {
                    if (*l1 == *l3) {
                        result->erase(l3);
                        break;
                    }
                    l3++;
                }
                break;
            }
            l2++;
        }
        l1++;
    }
}
