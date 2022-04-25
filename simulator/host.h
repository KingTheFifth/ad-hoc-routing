#ifndef HOST_H
#define HOST_H

#include <vector>
#include <queue>
#include <QGraphicsScene>
#include "link.h"
#include "packet.h"
#include "point.h"

using namespace std;

//class Link;

class Host {
    public:
        Host(int _x, int _y, int _radius)
        : point(_x, _y), radius(_radius) {}

        vector<Link*> neighbours;
        queue<Packet*> buffer;

        /*
        Adds all found neighbours to 'hosts'.
        */
        void discoverNeighbours(vector<Host*>* hosts);

        void addNeighbour(Host* host);

        Point getPos();

        void draw(QGraphicsScene *scene) const;

        void tick();

    private:
        Point point;
        int radius;
};

#endif