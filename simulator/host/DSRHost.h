#ifndef DSRHOST_H
#define DSRHOST_H

#include "host.h"
#include "packet/DSRPacket.h"
#include "DSRRoute.h"
#include <utility>

struct DSRHost : public Host {
    public:
        DSRHost(double _x, double _y, int _radius, int _time, unsigned _id)
            : Host(_x, _y, _radius, _time, _id), requestIDCounter(0) {}
        // void tick(int currTime);

        ~DSRHost() = default;

    protected:
        void processPacket(Packet* packet);
    private:
        unsigned requestIDCounter;
        vector<DSRRoute*> routes;
        vector<DSRPacket*> waitingForRouteBuffer;
        vector<pair<const Host*, unsigned>> recentlySeenRequests;

        /**
         * 
         */
        Link* DSR(DSRPacket* packet);

        Link* getCachedRoute(const Host* target);

        bool shouldBeDropped(const DSRPacket* packet);
};

#endif