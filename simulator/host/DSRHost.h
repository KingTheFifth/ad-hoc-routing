#ifndef DSRHOST_H
#define DSRHOST_H

#include "host.h"
#include "packet/DSRPacket.h"
#include "DSRRoute.h"
#include <utility>

struct DSRHost : public Host {
    public:
        DSRHost(StatisticsHandler* _statistics, double _x, double _y, int _radius, int _time, unsigned _id)
            : Host(_statistics, _x, _y, _radius, _time, _id), requestIDCounter(0) {}
        // void tick(int currTime);

        ~DSRHost() = default;

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

        void tick(int currTime);

        /**
         * Run the DSR algorithm and return the link for the packet to route across
         */
        Link* DSR(DSRPacket* packet);

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

        void countPacketDrop(Packet* packet);

        void deleteRoutes(Host* destination);

        void sendRERR(DSRPacket* packet);
};

#endif