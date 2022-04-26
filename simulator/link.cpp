#include "link.h"
#include "host.h"

Link::Link(Host *hostA, Host *hostB) {
    hosts.first = hostA;
    hosts.second = hostB;
    length = hostA->getPos().distanceTo(hostB->getPos());
}

void Link::draw(QGraphicsScene *scene) { 
    hosts.first->getPos().drawTo(hosts.second->getPos(), scene);
}

Host* Link::getOtherHost(Host *currentHost) {
    return hosts.first == currentHost ? hosts.second : hosts.first;
}

void Link::forwardPacket(Packet *packet) {
    pair<Packet*, int> transmission = make_pair(packet, length);
    linkBuffer.push_back(transmission);
}

void Link::tick(int timeDelta) {
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