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
        : radius(_radius), time(_time), id(_id), mobilityTarget(nullptr), processingPacket(nullptr), transmittingPacket(nullptr) {
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
        Packet* processingPacket;
        Packet* transmittingPacket;
        unsigned processingCountdown;
        unsigned transmittingCountdown;

        // DSR specific
        struct DSRRoute {
            vector<Host*> route;

            // Returns true if the given host is somehow reachable through this route
            bool hasTarget(Host* target) {
                return false;
            }

            // Returns the next hop host from the given source host according to this route.
            // Works under the assumption that a route does not necessarily start with the 
            // node that is forwarding a packet along a route.
            Host* getNextHop(Host* source) {
                return nullptr;
            }
        }
        vector<DSRRoute*> routes;

        // DSDV specific
        struct DSDVTable {
            unsigned tableVersion = 0; // TODO: set init value at a better time?

            // nextHop [Link*]
            // destination [Host*]
            // tableVersion [unsigned], (of the relevant host we got the update from)
            // cost [unsigned], (amount of hops)
        }

        // debug
        int perimDrawCountdown = 0; // remove sometime later

        /**
         * 
         */
        void getPerimeterLinks(vector<Link*>* result);

        /**
         * 
         */
        Link* getRHREdge(const Point* referencePoint, vector<Link*>* perimeterLinks) const;
};

#endif