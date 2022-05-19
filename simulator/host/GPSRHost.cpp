#include "GPSRHost.h"
#include "link.h"

GPSRHost::~GPSRHost() {
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

void GPSRHost::processPacket(Packet* packet) {
    // Either packet has arrived at the destination or the packet is processed and later forwarded
    
    GPSRPacket* gpsrPacket = (GPSRPacket*) packet;
    if (location->distanceTo(gpsrPacket->destPos) < CLOSE_THRESHOLD) {
        int delay = time - gpsrPacket->timeSent;
        statistics->addPacketArrival(delay);
        delete gpsrPacket;
    }
    else {
        Link* l = GPSR(gpsrPacket);
        if (l != nullptr) {
            gpsrPacket->prevPos = location;
            forwardPacket(gpsrPacket, l);
        }
        else {
            dropReceivedPacket(gpsrPacket);
        }
    }
}

Link* GPSRHost::GPSR(GPSRPacket* packet) {
    Point* previous = packet->prevPos;
    const Point* destination = packet->destPos;

    // Swap to Greedy mode if we are closer to the destination now than from the point of Greedy failure
    if (packet->mode == GPSRPacket::Perimeter &&
        location->distanceTo(destination) < packet->failurePos->distanceTo(destination)) {
        packet->mode = GPSRPacket::Greedy;
    }

    // Find the neighbour geographically closest to the destination
    Point* closestPos = location;
    Link* nextHopLink = nullptr;
    if (packet->mode == GPSRPacket::Greedy) {
        for (Link* link : neighbours) {
            Point* pos = link->getOtherHost(this)->getPos();
            if (pos->distanceTo(destination) < closestPos->distanceTo(destination)) {
                closestPos = pos;
                nextHopLink = link;
            }
        }
    }

    // If no geographically closer neighbour is found, switch to Perimeter routing
    if (closestPos->distanceTo(location) == 0 && packet->mode == GPSRPacket::Greedy) {
        packet->mode = GPSRPacket::Perimeter;
        packet->failurePos = location;

        vector<Link*> perimeter;
        getPerimeterLinks(&perimeter);
        nextHopLink = getRHREdge(destination, &perimeter);
        if(nextHopLink){
            packet->firstEdgeInPerim = make_pair(this, nextHopLink->getOtherHost(this));
        } else {
            return nullptr;
        }

    }
    else if (packet->mode == GPSRPacket::Perimeter) {
        vector<Link*> perimeter;
        getPerimeterLinks(&perimeter);
        Link* RHREdge = getRHREdge(previous, &perimeter);
        
        // Drop packet if host is isolated
        if (!RHREdge) return nullptr;
        Point* Lf = getCrossing(location, RHREdge->getOtherHost(this)->getPos(), packet->failurePos, destination);

        if (
            Lf &&
            (!(packet->destLineIntersect) ||
            Lf->distanceTo(destination) < packet->destLineIntersect->distanceTo(destination))
            ) {
            packet->destLineIntersect = Lf;
            Point* newRHRReference = RHREdge->getOtherHost(this)->getPos();
            RHREdge = getRHREdge(newRHRReference, &perimeter);
            if(RHREdge == nullptr){ return nullptr; }
            packet->firstEdgeInPerim = make_pair(this, RHREdge->getOtherHost(this));
        }

        // Drop packet if whole perimeter is traversed (i.e. destination could not be reached)
        if (this == packet->firstEdgeInPerim.first && RHREdge->getOtherHost(this) == packet->firstEdgeInPerim.second) return nullptr;
        nextHopLink = RHREdge; 
    }
    return nextHopLink;
}

// Can return null if this host is isolated
Link* GPSRHost::getRHREdge(const Point* referencePoint, vector<Link*>* perimeterLinks) const {
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

void GPSRHost::getPerimeterLinks(vector<Link*>* result) {
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

void GPSRHost::dropReceivedPacket(Packet* packet) {
    countPacketDrop(packet);
    delete packet;
}

void GPSRHost::countPacketDrop(Packet* packet) { // GPSR is too elite for parameters
    statistics->dropDataPacket();
}

void GPSRHost::deleteRoutes(Host* destination) {
    return;
}
