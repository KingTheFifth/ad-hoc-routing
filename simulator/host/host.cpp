#include "constants.h"
#include "link.h"
#include "host.h"
#include "packet/packet.h"
#include <cmath>

Host::~Host() {
    delete location;
    if (mobilityTarget) {
        delete mobilityTarget;
    }
}

void Host::discoverNeighbours() {
    for (auto it : *hosts) { // For all hosts in the topology
        double distance = location->distanceTo(it.second->location);
        if (it.second != this && distance < radius) { // If they are within the radius of this host
            addNeighbour(it.second); // Add them as a neighbour and create a link between them
        }
    }
}

void Host::addNeighbour(Host* host) {
    if(!isNeighbour(host)){ // If the host is not already a neighbour
        // Create a link between this host and 'host'
        Link* newLink = new Link(this, host, time);
        neighbours.push_back(newLink);
        host->neighbours.push_back(newLink);
    }
}

void Host::deleteNeighbour(Link* link){
    if(link != nullptr){ //Check if link still exists
        deleteRoutes(link->getOtherHost(this));
        if (link->isBroken) { // Neighbour has already deleted their side
            vector<Packet*> linkPackets;
            link->getPackets(&linkPackets);
            for(Packet* p : linkPackets) {
                countPacketDrop(p);
            }

            delete link;
        }
        else { // Neighbour needs to delete their side
            link->isBroken = true;
        }
    }
}

bool Host::isNeighbour(Host* host) {
    for (auto& link : neighbours) {
        if (link != nullptr && link->getOtherHost(this) == host) return true; // Check for nullptr
    }
    return false;
}

Point* Host::getPos() const {
    return location;
}

double Host::distanceTo(Host* host) const {
    return getPos()->distanceTo(host->getPos());
}

void Host::draw(QGraphicsScene *scene) const {
    location->draw(scene);
    for (auto& neighbour : neighbours) {
        neighbour->draw(scene);
    }
}

void Host::tick(int currTime) {
    // Calculate new time
    int timeDelta = currTime - time;
    time = currTime;

    vector<Link*>::iterator link_it = neighbours.begin();
    while (link_it != neighbours.end()) { // Loop through all neighbouring links
        Link* link = *link_it;
        if (link->getLength() > radius) { // If the link is too long
            deleteNeighbour(link); // Break it
            link_it = neighbours.erase(link_it);
        }
        else {
            link->tick(currTime); // Tick forward the links
            link_it++;
        }
    }
    
    // Start processing packet from buffer
    if (processingCountdown <= 0 && !receivingBuffer.empty()) {

        Packet* packet = receivingBuffer.front();
        receivingBuffer.pop();
        
        processPacket(packet);

        // Restart processing countdown
        processingCountdown = HOST_PROCESSING_DELAY;
    }

    if (processingCountdown > 0 && !receivingBuffer.empty()) {
        processingCountdown -= timeDelta;
    }
    
    if (transmitCountdown <= 0 && !transmitBuffer.empty()) {
        pair<Packet*, Link*> packetLink = transmitBuffer.front();
        transmitBuffer.pop();

        // If link was broken while a packet was in transmit buffer, drop the packet
        bool isBroken = true;
        for (Link* l : neighbours) {
            if (l == packetLink.second) isBroken = false;
        }
        if (!isBroken) {
            transmitPacket(packetLink.first, packetLink.second);
        }
        else {
            dropReceivedPacket(packetLink.first);
        }
        transmitCountdown = HOST_TRANSMISSION_DELAY;
    }

    if (transmitCountdown > 0 && !transmitBuffer.empty()) {
        transmitCountdown -= timeDelta;
    }

    if (HOST_MOBILITY && mobilityTarget != nullptr) { // If this host wants to move
        if (location->distanceTo(mobilityTarget) < CLOSE_THRESHOLD) { // Check if we have already moved close enough
            delete mobilityTarget;
            mobilityTarget = nullptr;
            discoverNeighbours();
        }
        else if (time % (HOST_MOVEMENT_SPEED * TICK_STEP) == 0) { // Else, update the position toward the target on set intervals
            // Move towards mobilityTarget
            double step = HOST_MOVEMENT_STEP;
            double angle = location->angleTo(mobilityTarget);
            double dy = sin(angle) * step;
            double dx = cos(angle) * step;

            location->y += dy;
            location->x += dx;

            discoverNeighbours();
        }
    }
    
}

void Host::forwardPacket(Packet *packet, Link *link) {
    packet->nextHop = link->getOtherHost(this);
    transmitBuffer.push(make_pair(packet, link));
}

void Host::transmitPacket(Packet *packet, Link *link) {
    link->forwardPacket(packet);
}

void Host::receivePacket(Packet* packet) {
    packet->ttl -= 1;
    if (receivingBuffer.size()*PACKET_SIZE >= HOST_BUFFER_SIZE || packet->ttl <= 0) {
        // Drop packet
        dropReceivedPacket(packet);
    } else {
        receivingBuffer.push(packet);
    }
}

void Host::moveTo(Point* target) {
    mobilityTarget = target;
}

bool Host::isIdle() {
    return receivingBuffer.empty() && transmitBuffer.empty();
}

void Host::broadcast(Packet* packet) {
    for (Link* l : neighbours) {
        forwardPacket(packet->copy(), l);
    }
}

Link* Host::getLinkToHost(const Host* target) {
    for (Link* l : neighbours) {
        if (l != nullptr && l->getOtherHost(this) == target) return l;
    }
    return nullptr;
}

void Host::die() {
    for (auto& link : neighbours) { // Remove all links to this host
        deleteNeighbour(link);
    }
}
