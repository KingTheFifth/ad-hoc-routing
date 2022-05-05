#include "link.h"
#include "host/host.h"
#include <QGraphicsRectItem>
#include "constants.h"
#include "packet/GPSRPacket.h" // debug

Link::Link(Host *hostA, Host *hostB, int currTime) {
    hosts.first = hostA;
    hosts.second = hostB;
    // length = hostA->getPos()->distanceTo(hostB->getPos()) / WINDOW_SCALE;
    time = currTime;
}

void Link::draw(QGraphicsScene *scene) { 
    hosts.first->getPos()->drawTo(hosts.second->getPos(), scene);
    for (vector<PacketOnLink*>::iterator it = linkBuffer.begin(); it != linkBuffer.end(); it++) {
        // TODO: extract into method :)

        Point* destination = (*it)->packet->nextHop->getPos();
        Point* source = getOtherHost((*it)->packet->nextHop)->getPos();
        double progress = (double) (*it)->timeOnLink * LINK_SPEED / (double) (*it)->origin->distanceTo((destination));

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

        QGraphicsRectItem *item = new QGraphicsRectItem(x * WINDOW_SCALE, y * WINDOW_SCALE, 3 * WINDOW_SCALE, 3 * WINDOW_SCALE); // TODO: This might be a memory leak, look up details
        item->setBrush(QBrush(QColor(0, 0, 255)));
        scene->addItem(item);

        // --- debug --- 
        // GPSRPacket* gpsrPacket = (GPSRPacket*) (*it)->packet;
        // const Point* d = gpsrPacket->destPos;
        // const Point* failure = gpsrPacket->failurePos;
        // if (d && failure)
        //     failure->drawToAsPerimeter(d, scene, true);
        // -------------
        
    }
}
void Link::drawAsPerimeter(QGraphicsScene *scene) { 
    hosts.first->getPos()->drawToAsPerimeter(hosts.second->getPos(), scene, false);
}

Host* Link::getOtherHost(const Host *currentHost) {
    return hosts.first == currentHost ? hosts.second : hosts.first;
}

void Link::forwardPacket(Packet *packet) {
    PacketOnLink* packetOnLink = new PacketOnLink();
    packetOnLink->packet = packet;
    packetOnLink->timeOnLink = 0;
    packetOnLink->origin = getOtherHost(packet->nextHop)->getPos();
    linkBuffer.push_back(packetOnLink);
}

void Link::tick(int currTime) {
    int timeDelta = currTime - time;
    if (timeDelta <= 0) return;
    time = currTime;
    
    vector<PacketOnLink*>::iterator it = linkBuffer.begin();
    while (it != linkBuffer.end()) {
        PacketOnLink* p = *it;
        p->timeOnLink += timeDelta;
        if (p->timeOnLink * LINK_SPEED >= p->origin->distanceTo(p->packet->nextHop->getPos())) {
            p->packet->nextHop->receivePacket(p->packet);
            it = linkBuffer.erase(it);
            delete p;
            continue;
        }
        ++it;
    }
}
