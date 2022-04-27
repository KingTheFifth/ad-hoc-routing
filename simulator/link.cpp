#include "link.h"
#include "host.h"
#include <QGraphicsRectItem>

Link::Link(Host *hostA, Host *hostB, int currTime) {
    hosts.first = hostA;
    hosts.second = hostB;
    length = hostA->getPos().distanceTo(hostB->getPos());
    time = currTime;
}

void Link::draw(QGraphicsScene *scene) { 
    hosts.first->getPos().drawTo(hosts.second->getPos(), scene);
    for (vector<pair<Packet*, int>>::iterator it = linkBuffer.begin(); it != linkBuffer.end(); it++) {
        double progress = 1 - ((double) it->second / (double) length); // 0 - 1        

        Point destination = it->first->nextHop->getPos();
        Point source = getOtherHost(it->first->nextHop)->getPos();
        int y = source.y;
        int x = source.x;
        int dx = destination.x - x;
        int dy = destination.y - y;
        double slope = source.slopeTo(destination);
        if (slope == -std::numeric_limits<double>::infinity())
            continue; // Don't bother drawing if the two hosts are overlapping
        else if (slope == std::numeric_limits<double>::infinity()) // vertical line
            y += (int) (dy * progress);
        else if (slope == 0) // horizontal line
            x += (int) (dx * progress);
        else { // diagonal line
            x += (int) (dx * progress);
            y += (int) (dy * progress);
        }

        QGraphicsRectItem *item = new QGraphicsRectItem(x, y, 2, 2);
        item->setBrush(QBrush(QColor(0, 0, 255)));
        scene->addItem(item);
        
    }
}

Host* Link::getOtherHost(Host *currentHost) {
    return hosts.first == currentHost ? hosts.second : hosts.first;
}

void Link::forwardPacket(Packet *packet) {
    pair<Packet*, int> transmission = make_pair(packet, length);
    linkBuffer.push_back(transmission);
}

void Link::tick(int currTime) {
    int timeDelta = currTime - time;
    if (timeDelta <= 0) return;
    time = currTime;
    
    vector<pair<Packet*, int>>::iterator it = linkBuffer.begin();
    while (it != linkBuffer.end()) {
        it->second -= timeDelta;
        if (it->second <= 0) {
            it->first->nextHop->receivePacket(it->first);
            it = linkBuffer.erase(it);
            continue;
        }
        ++it;
    }
}