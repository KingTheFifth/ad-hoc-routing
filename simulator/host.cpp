#include "constants.h"
#include "link.h"
#include "host.h"
#include "packet.h"
#include <iostream>

void Host::discoverNeighbours(vector<Host*>* hosts) {
    for (auto& host : *hosts) {
        double distance = getPos().distanceTo(host->point);
        if (host != this && distance < radius) {
            addNeighbour(host);
        }
    }
}

void Host::addNeighbour(Host* host) {
    for (auto& link : neighbours) {
        if (link->getOtherHost(this) == host) return;
    }
    
    Link* newLink = new Link(this, host);
    neighbours.push_back(newLink);
    host->neighbours.push_back(newLink);
}

Point Host::getPos() {
    return point;
}

void Host::draw(QGraphicsScene *scene) const {
    point.draw(scene);
    for (auto& neighbour : neighbours) {
        neighbour->draw(scene);
    }
}

void Host::tick() {
    // :)
    // if something, pop from receive-queue
}

void Host::forwardPacket(Packet *packet, Link *link) {
    link->forwardPacket(packet);
}

void Host::receivePacket(Packet* packet) {
    if (buffer.size()*PACKET_SIZE >= HOST_BUFFER_SIZE) {
        // drop packet
        delete packet; // :) ?
    } else {
        buffer.push(packet);
    }
}