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

    if (currTime % 5 == 0 && !buffer.empty()) { // temporary while routing is not implemented
        Packet* packet = buffer.front();
        buffer.pop();
        if (location->distanceTo(packet->destPos) < CLOSE_THRESHOLD) {
            // we have arrived :)
            cout << "Packet " << packet << " has arrived at host " << id << " at pos " << *(packet->destPos) << endl;
            delete packet;
        }
        else {
            Link* l = GPSR(packet);
            cout << "Forwarding onto link " << l << endl;
            if (l) // if host is isolated, do not send packet
                forwardPacket(packet, l);
            else {
                cout << "deleting" << endl;
                delete packet; // drop packet
            }
        }
    }
    /*
    if (buffer not empty && processing delay is over)
        do routing
    */
    
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
    cout << "Forwarding packet" << endl;
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
    cout << "packet " << packet <<  " - Doing GPSR with " << (packet->mode == Packet::Perimeter ? "PERIMETER" : "GREEDY") << " mode" << endl;
    Point* previous = packet->prevPos;
    const Point* destination = packet->destPos;

    // Swap to Greedy mode if we are closer to the destination now than from the point of Greedy failure
    if (packet->mode == Packet::Perimeter &&
        location->distanceTo(destination) < packet->failurePos->distanceTo(destination)) {
        cout << "packet " << packet << " - Swapping to Greedy mode" << endl;
        packet->mode = Packet::Greedy;
    }

    //cout << "Perimeter: " << (packet->mode == Packet::Perimeter) << ", Greedy: " << (packet->mode == Packet::Greedy) << endl;
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
        if (closestPos->distanceTo(location) != 0)
            cout << "packet " << packet << " - Found a closer neighbour in Greedy mode" << endl;
        else
            cout << "packet " << packet << " - Did not find a closer neighbour, soon switching to Perimeter" << endl;
    }

    // If no geographically closer neighbour is found, switch to Perimeter routing
    if (closestPos->distanceTo(location) == 0 && packet->mode == Packet::Greedy) {
        packet->mode = Packet::Perimeter;
        packet->failurePos = location;

        vector<Link*> perimeter;
        getPerimeterLinks(&perimeter);
        getPerimeterLinks(&perimeterLinks);
        nextHopLink = getRHREdge(destination, &perimeter);
        cout << "packet " << packet << " - Got Perimeter Links, and found the next hop link by RHR :)" << endl;
    }
    else if (packet->mode == Packet::Perimeter) {
        vector<Link*> perimeter;
        getPerimeterLinks(&perimeter);
        getPerimeterLinks(&perimeterLinks);
        cout << "Using prevPos: " << *previous << endl;
        Link* RHREdge = getRHREdge(previous, &perimeter);
        
        // Drop packet if host is isolated or whole perimeter is traversed (i.e. destination could not be reached)
        if (!RHREdge || this == packet->firstEdgeInPerim) return nullptr;
        Point* Lf = getCrossing(location, RHREdge->getOtherHost(this)->getPos(), packet->failurePos, destination);

        cout << "L_f: " << Lf << endl;
        bool thing = (Lf &&
            (!packet->destLineIntersect ||
            Lf->distanceTo(destination) < packet->destLineIntersect->distanceTo(destination)));
        cout << "if-condition: " << thing << endl;
        if (
            Lf &&
            (!packet->destLineIntersect ||
            Lf->distanceTo(destination) < packet->destLineIntersect->distanceTo(destination))
        ) {
            cout << "hej3" << endl;
            packet->destLineIntersect = Lf;
            Point* newRHRReference = RHREdge->getOtherHost(this)->getPos();
            RHREdge = getRHREdge(newRHRReference, &perimeter);
            packet->firstEdgeInPerim = RHREdge->getOtherHost(this);
            cout << "hej4" << endl;
        }
        nextHopLink = RHREdge; 
    }
    return nextHopLink;
}

// can return null if this host is isolated
Link* Host::getRHREdge(const Point* referencePoint, vector<Link*>* perimeterLinks) const {
    double referenceAngle = location->angleTo(referencePoint);
    cout << "Reference angle: " << referenceAngle * 180/PI << endl;
    double smallestAngle = std::numeric_limits<double>::infinity();
    Link* RHREdge = nullptr;
    for (Link* link : *perimeterLinks) {
        Point* perimNeighbourPos = link->getOtherHost(this)->getPos();
        double perimNeighbourAngle = location->angleTo(perimNeighbourPos);
        if (perimNeighbourAngle <= referenceAngle) perimNeighbourAngle += 2*PI;
        cout << "perimNeighbourAngle: " << perimNeighbourAngle * 180/PI << " (" << (perimNeighbourAngle > 2*PI ? (perimNeighbourAngle - 2*PI) * 180/PI : -1) << ")" << endl;
        
        double angleDiff = perimNeighbourAngle - referenceAngle;
        cout << "smallestAngle: "  << smallestAngle * 180/PI << endl;
        cout << "angleDiff: "  << angleDiff * 180/PI << endl;
        cout << "angleDiff < smallestAngle: "  << (angleDiff < smallestAngle) << endl;
        if (angleDiff < smallestAngle) {
            cout << "New smallest angle diff: " << angleDiff * 180/PI << endl;
            smallestAngle = angleDiff;
            RHREdge = link;
        }
    }
    return RHREdge;
}

void Host::getPerimeterLinks(vector<Link*>* result) {
    /*
        // Potential solution by adding instead of removing
        for (vector<Link*>::iterator l1 = neighbours.begin(); l1 != neighbours.end(); l1++) {
            for (vector<Link*>::iterator l2 = neighbours.begin(); l2 != neighbours.end(); l2++) {
                if (l1 == l2) continue;
                
                Host* n1 = (*l1)->getOtherHost(this);
                Host* n2 = (*l2)->getOtherHost(this);
                double distThisToN1 = getPos()->distanceTo(n1->getPos());
                double distThisToN2 = getPos()->distanceTo(n2->getPos());
                double distN1ToN2 = n1->getPos()->distanceTo(n2->getPos());
                
                if (distThisToN1 < max(distThisToN2, distN1ToN2)) {
                    result->push_back(*l1);
                    //break;
                }
            }
        }
    */
    
    // Copy the set of neighbours into result
    for (unsigned i = 0; i < neighbours.size(); i++) {
        result->push_back(neighbours[i]);
    }
    
    // Remove crossing links as per RNG
    vector<Link*>::iterator l1 = result->begin(); 
    while (l1 != result->end()) {
        vector<Link*>::iterator l2 = result->begin();
        while (l2 != result->end()) {
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
                l1 = result->erase(l1);
                l1--;
                break;
            }
            l2++;
        }
        l1++;
    }
}

void Host::getPerimeterLinksGG(vector<Link*>* result) {
    // Copy the set of neighbours into result
    for (unsigned i = 0; i < neighbours.size(); i++) {
        result->push_back(neighbours[i]);
    }
    
    vector<Link*>::iterator l1 = result->begin(); 
    while (l1 != result->end()) {
        Host* n1 = (*l1)->getOtherHost(this);
        
        // calculate the midpoint between this host and host n1
        double dx = (location->x - n1->getPos()->x) / 2.0;
        double dy = (location->y - n1->getPos()->y) / 2.0;
        Point mid = Point(location->x + dx, location->y + dy);

        vector<Link*>::iterator l2 = result->begin();
        while (l2 != result->end()) {
            if (l1 == l2) {
                l2++;
                continue;
            }

            Host* n2 = (*l2)->getOtherHost(this);

            if (mid.distanceTo(n2->getPos()) < location->distanceTo(&mid)) {
                l1 = result->erase(l1);
                l1--;
                break;
            }
            l2++;
        }
        l1++;
    }
}