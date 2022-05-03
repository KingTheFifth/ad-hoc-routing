#include "constants.h"
#include "link.h"
#include "host.h"
#include "packet/packet.h"
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
    /*if (perimDrawCountdown > 0) {
        for(auto& perimeter : perimeterLinks) {
            perimeter->drawAsPerimeter(scene);
        }
    }*/
}

void Host::tick(int currTime) {
    int timeDelta = currTime - time;
    time = currTime;
    
    if (perimDrawCountdown > 0) perimDrawCountdown--;

    for (auto& link : neighbours) {
        link->tick(currTime);
    }
    
    // Start processing packet from buffer
    if (processingCountdown <= 0 && !receivingBuffer.empty()) {
        // cout << "Starting processing" << endl;

        Packet* packet = receivingBuffer.front();
        receivingBuffer.pop();
        
        processPacket(packet);

        // Restart processing countdown
        processingCountdown = HOST_PROCESSING_DELAY;
    }
    if (processingCountdown > 0 && !receivingBuffer.empty()) {
        processingCountdown -= timeDelta;
    }
    
    if (transmitCountdown <= 0 && transmitBuffer.first != nullptr) {
        transmitPacket(transmitBuffer.first, transmitBuffer.second);
        transmitBuffer = make_pair(nullptr, nullptr);
    }
    if (transmitCountdown > 0) {
        transmitCountdown -= timeDelta;
    }

    if (mobilityTarget != nullptr) {
        if (location->distanceTo(mobilityTarget) < CLOSE_THRESHOLD) {
            mobilityTarget = nullptr;
        }
        else {
            // Move towards mobilityTarget
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
    packet->nextHop = link->getOtherHost(this);
    transmitBuffer = make_pair(packet, link);
    transmitCountdown = HOST_TRANSMISSION_DELAY;
}

void Host::transmitPacket(Packet *packet, Link *link) {
    // packet->prevPos = location;
    link->forwardPacket(packet);
}

void Host::receivePacket(Packet* packet) {
    if (receivingBuffer.size()*PACKET_SIZE >= HOST_BUFFER_SIZE) {
        // Drop packet
        delete packet;
    } else {
        receivingBuffer.push(packet);
    }
}

void Host::moveTo(Point* target) {
    mobilityTarget = target;
}

void Host::broadcast(Packet* packet) {
    for (Link* l : neighbours) {
        forwardPacket(packet->copy(), l); // TODO: This is broken
    }
}
