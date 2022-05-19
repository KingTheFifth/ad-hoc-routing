#ifndef DSRHOST_H
#define DSRHOST_H

#include "host.h"
#include "packet/DSRPacket.h"
#include "DSRRoute.h"
#include <utility>

struct DSRHost : public Host {
    public:
        DSRHost(StatisticsHandler* _statistics, double _x, double _y, int _radius, int _time, unsigned _id, unordered_map<unsigned, Host*>* _hosts)
            : Host(_statistics, _x, _y, _radius, _time, _id, _hosts), requestIDCounter(0) {}

        ~DSRHost();

        /**
         *
         */
        void die();

    protected:

        /**
         * 
         */
        void processPacket(Packet* packet);

        /**
         * 
         */
        void dropReceivedPacket(Packet* packet);

    private:
        unsigned requestIDCounter;
        vector<DSRRoute*> routes;
        vector<pair<DSRPacket*, int>> waitingForRouteBuffer;
        vector<pair<const Host*, unsigned>> recentlySeenRequests;

        /**
         *
         */
        void tick(int currTime);

        /**
         * Run the DSR algorithm and return the link for the packet to route across
         */
        void DSR(DSRPacket* packet);

        /**
         * 
         */
        Link* getCachedNextHop(const Host* target);

        /**
         * 
         */
        DSRRoute* getCachedRoute(const Host* target);

        /**
         * 
         */
        bool shouldBeDropped(const DSRPacket* packet);

        /**
         *
         */
        void countPacketDrop(Packet* packet);

        /**
         *
         */
        void deleteRoutes(Host* destination);

        /**
         *
         */
        void sendRERR(DSRPacket* packet);

        /**
         *
         */
        void handleRERR(DSRPacket* packet);

};

#endif