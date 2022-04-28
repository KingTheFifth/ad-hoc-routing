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

Point* Host::getPos() {
    return location;
}

void Host::draw(QGraphicsScene *scene) const {
    location->draw(scene);
    for (auto& neighbour : neighbours) {
        neighbour->draw(scene);
    }
}

void Host::tick(int currTime) {
    time = currTime;

    for (auto& link : neighbours) {
        link->tick(currTime);
    }

    if (currTime % 5 == 0 && !buffer.empty()) { // temporary while routing is not implemented
        Packet* packet = buffer.front();
        buffer.pop();
        int rndindex = rand() % neighbours.size();
        Link* l = neighbours[rndindex];

        forwardPacket(packet, l);
        
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
            int dy = (int) (sin(angle) * step);
            int dx = (int) (cos(angle) * step);

            location->y += dy;
            location->x += dx;
        }
    }
    
}

void Host::forwardPacket(Packet *packet, Link *link) {
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

Link* Host::DSR(Host* destination) {

}

Link* Host::DSDV(Host* destination) {

}

Link* Host::GPSR(Point* destination) {
    // Find the neighbour geographically closest to the destination
    Point* closestPos = location;
    Link* closestLink = NULL;
    for (Link* link : neighbours) {
        Point* pos = link->getOtherHost(this)->getPos();
        if (pos->distanceTo(destination) < closestPos->distanceTo(destination)) {
            closestPos = pos;
            closestLink = link;
        }
    }

    if (closestPos->distanceTo(location) == 0) { // If no geographically closer neighbour is found, we switch to perimeter routing
        double angleToDestination = location->angleTo(destination);
        double smallestAngle = 1; //TO-DO: replace 1 with actual stuff 
        for (Link* link : neighbours) {
            Point* neighbourPos = link->getOtherHost(this)->getPos();
            double linkAngle = location->angleTo(neighbourPos) - angleToDestination;
            if (linkAngle < 0) { linkAngle += (PI * 2);}
            if (linkAngle < smallestAngle) {
                smallestAngle = linkAngle;
                closestLink = link;
            }
        }
    }
    return closestLink;
}
