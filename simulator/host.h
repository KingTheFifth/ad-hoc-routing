#ifndef HOST_H
#define HOST_H

#include <vector>
#include <queue>
#include <QGraphicsScene>
//#include "link.h"
#include "packet.h"
#include "point.h"

using namespace std;

class Link;

class Host {
    public:
        Host(double _x, double _y, int _radius, int _time, unsigned _id)
        : radius(_radius), time(_time), id(_id), mobilityTarget(nullptr) {
            location = new Point(_x, _y);
        }

        vector<Link*> neighbours;
        queue<Packet*> buffer;

        /**
         * Adds all found neighbours to 'hosts'.
         */
        void discoverNeighbours(vector<Host*>* hosts);

        /**
         * 
         */
        void addNeighbour(Host* host);

        /**
         * 
         */
        Point* getPos() const;

        /**
         * 
         */
        void draw(QGraphicsScene *scene) const;

        /**
         * 
         */
        void tick(int currTime);

        /**
         * 
         */
        void forwardPacket(Packet* packet, Link* link);

        /**
         * 
         */
        void receivePacket(Packet* packet);

        /**
         * 
         */
        void moveTo(Point* target);

        /**
         * 
         */
        Link* DSR(Packet* packet);

        /**
         * 
         */
        Link* DSDV(Packet* packet);

        /**
         * 
         */
        Link* GPSR(Packet* packet);

    private:
        Point* location;
        int radius;
        int time;
        unsigned id;
        Point* mobilityTarget;
        vector<Link*> perimeterLinks;
        int perimDrawCountdown = 0; // remove sometime later

        /**
         * 
         */
        void getPerimeterLinks(vector<Link*>* result);
        void getPerimeterLinksGG(vector<Link*>* result);

        /**
         * 
         */
        Link* getRHREdge(const Point* referencePoint, vector<Link*>* perimeterLinks) const;
};

#endif