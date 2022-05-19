#ifndef HOST_H
#define HOST_H

#include <unordered_map>
#include <queue>
#include <QGraphicsScene>
#include <utility>
//#include "link.h"
#include "packet/packet.h"
#include "StatisticsHandler.h"
#include "point.h"
#include "constants.h"

using namespace std;

class Link;

class Host {
    public:
        Host(StatisticsHandler* _statistics, double _x, double _y, int _radius, int _time, unsigned _id, unordered_map<unsigned, Host*>* _hosts)
        : id(_id), statistics(_statistics), radius(_radius), time(_time), mobilityTarget(nullptr), processingCountdown(HOST_PROCESSING_DELAY), transmitCountdown(0), hosts(_hosts) {
            location = new Point(_x, _y);
        }

        virtual ~Host();

        vector<Link*> neighbours;
        queue<Packet*> receivingBuffer;
        unsigned id;

        /**
         * 
         */
        void discoverNeighbours();

        /**
         * Add a neighbour 'host' to this host
         */
        void addNeighbour(Host* host);

        /**
         * 
         */
        void deleteNeighbour(Link* link);

        /**
         * Return true if 'host' is a neighbour of this host
         */
        bool isNeighbour(Host* host);

        /**
         * Get the position of this host
         */
        Point* getPos() const;

        /**
         * Get the distance from this host to 'host'
         */
        double distanceTo(Host* host) const;

        /**
         * Draw this host and all the neighbouring links
         */
        void draw(QGraphicsScene *scene) const;

        /**
         * Tick this host forward one simulation tick
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
         * Receive a packet and handle it depending on if it needs to be retransmitted or if it has arrived
         */
        void receivePacket(Packet* packet);

        /**
         * Move this host to the new location 'target'
         */
        void moveTo(Point* target);

        /** 
         * A host is idle if it has no packet in its receivingBuffer nor its transmitBuffer
         */
        bool isIdle();

        /**
         *
         */
        virtual void countPacketDrop(Packet* packet) = 0;

        /**
         *
         */
        virtual void deleteRoutes(Host* destination) = 0;

        /**
         *
         */
        virtual void die();

    protected:
        StatisticsHandler* statistics;
        Point* location;
        int radius;
        int time;
        Point* mobilityTarget;
        queue<pair<Packet*, Link*>> transmitBuffer;
        int processingCountdown;
        int transmitCountdown;
        unordered_map<unsigned, Host*>* hosts;

        /**
         * Sends a copy of 'packet' to every neighbour of this host.
         */
        void broadcast(Packet* packet);

        /**
         * Handle the processing of 'packet'
         */
        virtual void processPacket(Packet* packet) = 0;

        /**
         * Get the link to 'target'
         */
        Link* getLinkToHost(const Host* target);

        /**
         * Drop 'packet'
         */
        virtual void dropReceivedPacket(Packet* packet) = 0;

        
};

#endif
