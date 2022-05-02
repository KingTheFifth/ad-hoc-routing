#ifndef HOST_H
#define HOST_H

#include <vector>
#include <queue>
#include <QGraphicsScene>
#include <utility>
//#include "link.h"
#include "packet/packet.h"
#include "point.h"
#include "constants.h"

using namespace std;

class Link;

class Host {
    public:
        Host(double _x, double _y, int _radius, int _time, unsigned _id)
        : radius(_radius), time(_time), id(_id), mobilityTarget(nullptr), processingCountdown(HOST_PROCESSING_DELAY) {
            location = new Point(_x, _y);
        }

        virtual ~Host() = default;

        vector<Link*> neighbours;
        queue<Packet*> receivingBuffer;

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
        virtual void tick(int currTime);

        /**
         * Given a packet and a link begins transmitting the packet over the link. The actual sending of the packet 
         * is done after HOST_TRANSMISSION_DELAY ms, in the transmitPacket method.
         */
        void forwardPacket(Packet* packet, Link* link);

        /**
         * This method actually transmits the packet
         */
        void transmitPacket(Packet* packet, Link* link);

        /**
         * 
         */
        void receivePacket(Packet* packet);

        /**
         * 
         */
        void moveTo(Point* target);

    protected:
        Point* location;
        int radius;
        int time;
        unsigned id;
        Point* mobilityTarget;
        pair<Packet*, Link*> transmitBuffer;
        int processingCountdown;
        int transmitCountdown;

        // debug
        int perimDrawCountdown = 0; // TODO: remove sometime later

        /**
         * Sends a copy of the packet to every neighbour of this host.
         */
        void broadcast(Packet* packet);

        virtual void processPacket(Packet* packet) = 0;
};

#endif