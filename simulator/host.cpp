#include "host.h"

// Host::Host(int _x, int _y, int _radius)
//     : point(_x, _y), radius(_radius) {}

void Host::discoverNeighbours(vector<Host*>* hosts) {
    for (auto& host : *hosts) {
        double distance = host->getPos().distanceTo(host->point);
        if (distance < radius) {
            addNeighbour(host);
        }
    }
}

void Host::addNeighbour(Host* host) {
    Link* newLink = new Link(this, host);
    neighbours.push_back(newLink);
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