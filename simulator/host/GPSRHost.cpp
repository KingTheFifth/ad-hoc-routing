#include "GPSRHost.h"
#include "link.h"

GPSRHost::~GPSRHost() {
    while (!transmitBuffer.empty()) { // Empty the transmitBuffer
        Packet* p = transmitBuffer.front().first;
        transmitBuffer.pop();
        dropReceivedPacket(p);
    }

    while (!receivingBuffer.empty()) { // Empty the receivingBuffer
        Packet* p = receivingBuffer.front();
        receivingBuffer.pop();
        dropReceivedPacket(p);
    }
}

void GPSRHost::processPacket(Packet* packet) {
    // Either 'packet' has arrived at the destination or the packet is processed and later forwarded
    GPSRPacket* gpsrPacket = (GPSRPacket*) packet;
    if (location->distanceTo(gpsrPacket->destPos) < CLOSE_THRESHOLD) { // If the packet is at its destination
        int delay = time - gpsrPacket->timeSent;
        statistics->addPacketArrival(delay);
        delete gpsrPacket;
    }
    else { // If the packet is not yet at its destination
        Link* l = GPSR(gpsrPacket);
        if (l != nullptr) { // Forward the packet if we have found a link to follow
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

    Point* closestPos = location;
    Link* nextHopLink = nullptr;
    if (packet->mode == GPSRPacket::Greedy) { // Forward greedily if the packet is in Greedy mode
        // Find the neighbour geographically closest to the destination
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
    else if (packet->mode == GPSRPacket::Perimeter) { // If the packet is already in Perimeter mode
        vector<Link*> perimeter;
        getPerimeterLinks(&perimeter);
        Link* RHREdge = getRHREdge(previous, &perimeter);
        
        // Drop packet if this host is isolated
        if (!RHREdge) return nullptr;
        Point* Lf = getCrossing(location, RHREdge->getOtherHost(this)->getPos(), packet->failurePos, destination);

        // If the line from the point where we switched to Perimeter mode to the destination crosses the first 
        // right-hand-rule-derived edge, and this intersection point is closer to the destination than any earlier such 
        // intersection point, it means we are in a "new" perimeter that we should route around the edge of.
        // In that case, the right-hand-rule is applied with the first right-hand-rule-derived edge as the reference 
        // point, giving us the edge to route to.
        if (Lf &&
            (!(packet->destLineIntersect) ||
            Lf->distanceTo(destination) < packet->destLineIntersect->distanceTo(destination)))
        {
            packet->destLineIntersect = Lf;

            Point* newRHRReference = RHREdge->getOtherHost(this)->getPos();
            RHREdge = getRHREdge(newRHRReference, &perimeter);
            if(RHREdge == nullptr){ return nullptr; }
            
            // Record the start of this new perimeter in order to later know if the packet has been routed the whole 
            // way around the perimeter
            packet->firstEdgeInPerim = make_pair(this, RHREdge->getOtherHost(this));
        }

        // Drop packet if whole perimeter is traversed (i.e. destination could not be reached)
        if (this == packet->firstEdgeInPerim.first && RHREdge->getOtherHost(this) == packet->firstEdgeInPerim.second) return nullptr;
        nextHopLink = RHREdge; 
    }
    return nextHopLink;
}

Link* GPSRHost::getRHREdge(const Point* referencePoint, vector<Link*>* perimeterLinks) const {
    // Set defaults
    double referenceAngle = location->angleTo(referencePoint);
    double smallestAngle = std::numeric_limits<double>::infinity();
    Link* RHREdge = nullptr;
    
    for (Link* link : *perimeterLinks) { // For all perimeter links of this planarised graph
        // Calculate the angle to the neighbour across the link
        Point* perimNeighbourPos = link->getOtherHost(this)->getPos();
        double perimNeighbourAngle = location->angleTo(perimNeighbourPos);
        if (perimNeighbourAngle <= referenceAngle) perimNeighbourAngle += 2*PI; // If the angle delta compared to the referenceAngle is negative, make it a low priority
        
        double angleDiff = perimNeighbourAngle - referenceAngle;
        if (angleDiff < smallestAngle) { // Set the smallestAngle as the edge with highest priority (lowest angle compared to reference angle)
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
    while (l1 != neighbours.end()) { // For all neighbours
        vector<Link*>::iterator l2 = neighbours.begin();
        while (l2 != neighbours.end()) { // Compare them to each other
            if (l1 == l2) { // Do not compare a neighbouring link to itself
                l2++;
                continue;
            }
            
            Host* n1 = (*l1)->getOtherHost(this);
            Host* n2 = (*l2)->getOtherHost(this);
            double distThisToN1 = location->distanceTo(n1->getPos()); // Distance from this host to Neighbour 1
            double distThisToN2 = location->distanceTo(n2->getPos()); // Distance from this host to Neighbour 2
            double distN1ToN2 = n1->getPos()->distanceTo(n2->getPos()); // Distance from Neighbour 1 to Neighbour 2
            
            if (distThisToN1 > max(distThisToN2, distN1ToN2)) { // If 'distThisToN1' is larger than the largest of 'distThisToN2' and 'distN1ToN2'
                vector<Link*>::iterator l3 = result->begin();
                while (l3 != result->end()) { // Find the link to the first neighbour again (as to avoid breaking l1's iteration)
                    if (*l1 == *l3) {
                        result->erase(l3); // And remove it from 'result'
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
