#include "link.h"
#include "host.h"

Link::Link(Host *hostA, Host *hostB) {
    hosts.first = hostA;
    hosts.second = hostB;
    length = hostA->getPos().distanceTo(hostB->getPos());
}

void Link::draw(QGraphicsScene *scene) { 
    if (!isDrawn) {
        isDrawn = true;
        hosts.first->getPos().drawTo(hosts.second->getPos(), scene);
    }
}