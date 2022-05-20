#include "link.h"
#include "host/host.h"
#include <QGraphicsRectItem>
#include "constants.h"

Link::Link(Host *hostA, Host *hostB, int currTime) {
    hosts.first = hostA;
    hosts.second = hostB;
    time = currTime;
    isBroken = false;
}

Link::~Link() {
    for (PacketOnLink* p : linkBuffer) {
        delete p->packet;
        delete p;
    }
}

void Link::draw(QGraphicsScene *scene) { 
    hosts.first->getPos()->drawTo(hosts.second->getPos(), scene); // Draw out the link

    // For all packets on the link
    for (vector<PacketOnLink*>::iterator it = linkBuffer.begin(); it != linkBuffer.end(); it++) {
        // Calculate the progress the packet has made to the other host
        Point* destination = (*it)->packet->nextHop->getPos();
        Point* source = getOtherHost((*it)->packet->nextHop)->getPos();
        double progress = (double) (*it)->timeOnLink * LINK_SPEED / (double) (*it)->origin->distanceTo((destination));

        // Calculate the current position of the packet
        double y = source->y;
        double x = source->x;
        double dx = destination->x - x;
        double dy = destination->y - y;
        double slope = source->slopeTo(destination);
        if (slope == -std::numeric_limits<double>::infinity())
            continue; // Don't bother drawing if the two hosts are overlapping
        else if (slope == std::numeric_limits<double>::infinity()) // vertical line
            y += dy * progress;
        else if (slope == 0) // horizontal line
            x += dx * progress;
        else { // diagonal line
            x += dx * progress;
            y += dy * progress;
        }

        QGraphicsRectItem *item;
        item = new QGraphicsRectItem(x * WINDOW_SCALE, y * WINDOW_SCALE, 6 * WINDOW_SCALE, 6 * WINDOW_SCALE);
        item->setBrush(QBrush(QColor((*it)->packet->color)));
        scene->addItem(item); // Draw the packet on the link
    }
}

Host* Link::getOtherHost(const Host *currentHost) {
    return hosts.first == currentHost ? hosts.second : hosts.first;
}

double Link::getLength() {
    if (isBroken) return std::numeric_limits<double>::infinity(); // If the link is broken, it has no valid length
    return hosts.first->getPos()->distanceTo(hosts.second->getPos());
}

void Link::getPackets(vector<Packet*>* resultVector) {
    for (PacketOnLink* pOnLink : linkBuffer) {
        resultVector->push_back(pOnLink->packet);
    }
}

void Link::forwardPacket(Packet *packet) {
    PacketOnLink* packetOnLink = new PacketOnLink(); // Create a PacketOnLink object for 'packet'
    packetOnLink->packet = packet;
    packetOnLink->timeOnLink = 0;
    packetOnLink->origin = getOtherHost(packet->nextHop)->getPos();
    linkBuffer.push_back(packetOnLink);
}

void Link::tick(int currTime) {
    // Update the time for this link
    int timeDelta = currTime - time;
    if (timeDelta <= 0) return; // If there is no difference in time, the simulation have not yet moved forward
    time = currTime;
    
    vector<PacketOnLink*>::iterator it = linkBuffer.begin();
    while (it != linkBuffer.end()) { // For all packets on the link
        PacketOnLink* p = *it;
        p->timeOnLink += timeDelta;

        // If the packet has reached its destination
        if (p->timeOnLink * LINK_SPEED >= p->origin->distanceTo(p->packet->nextHop->getPos())) { 
            p->packet->nextHop->receivePacket(p->packet); // Receive the packet on the other end of the link
            it = linkBuffer.erase(it); // Remove the PacketOnLink object created for this packet
            delete p;
            continue;
        }
        ++it;
    }
}
