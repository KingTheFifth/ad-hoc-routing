#include "link.h"
#include "host/host.h"
#include <QGraphicsRectItem>

Link::Link(Host *hostA, Host *hostB, int currTime) {
    hosts.first = hostA;
    hosts.second = hostB;
    length = hostA->getPos()->distanceTo(hostB->getPos()) / WINDOW_SCALE;
    time = currTime;
}

void Link::draw(QGraphicsScene *scene) { 
    hosts.first->getPos()->drawTo(hosts.second->getPos(), scene);
    for (vector<pair<Packet*, int>>::iterator it = linkBuffer.begin(); it != linkBuffer.end(); it++) {
        // TODO: extract into method :)
        double progress = 1 - ((double) it->second / (double) length);    

        Point* destination = it->first->nextHop->getPos();
        Point* source = getOtherHost(it->first->nextHop)->getPos();
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

        /*
        const Point* d = it->first->destPos;
        const Point* failure = it->first->failurePos;
        if (d && failure)
            failure->drawToAsPerimeter(d, scene, true);
        */
    }
}
void Link::drawAsPerimeter(QGraphicsScene *scene) { 
    hosts.first->getPos()->drawToAsPerimeter(hosts.second->getPos(), scene, false);
}

Host* Link::getOtherHost(const Host *currentHost) {
    return hosts.first == currentHost ? hosts.second : hosts.first;
}

void Link::forwardPacket(Packet *packet) {
    // cout << "Forwarding packet in link" << endl;
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
