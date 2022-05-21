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
         * Disconnect this host from the network, removing all packets it had in its buffers
         */
        void die();

    protected:

        /**
         * Specific processPacket function for DSR, process 'packet' according to the DSR routing algorithm
         */
        void processPacket(Packet* packet);

        /**
         * Drop 'packet'. Count the packet as a dropped data packet if it was one
         */
        void dropReceivedPacket(Packet* packet);

    private:
        unsigned requestIDCounter;
        vector<DSRRoute*> routes;
        vector<pair<DSRPacket*, int>> waitingForRouteBuffer;
        vector<pair<const Host*, unsigned>> recentlySeenRequests;

        /**
         * Tick the simulation forward once, forwarding packets if appropriate
         */
        void tick(int currTime);

        /**
         * Run the DSR algorithm and return the link for the packet to route across
         */
        void DSR(DSRPacket* packet);

        /**
         * Get the next link to follow if we want to route to 'target'. 
         * Can return nullptr if the link has been broken or if there was no cached route to 'target'
         */
        Link* getCachedNextHop(const Host* target);

        /**
         * Get the cached route in this host to 'target'. If no route is found, return nullptr
         */
        DSRRoute* getCachedRoute(const Host* target);

        /**
         * Determine wether a packet should be dropped or not. 
         * Used when checking if a specific RREQ packet has already been at this host
         */
        bool shouldBeDropped(const DSRPacket* packet);

        /**
         * Count 'packet' as a data packet dropped if it is one
         */
        void countPacketDrop(Packet* packet);

        /**
         * Not used in DSR
         */
        void deleteRoutes(Host* destination);

        /**
         * Used if a packet has failed to route because of link breakage. 
         * Send out a RERR packet letting the sending host know of the broken link
         */
        void sendRERR(DSRPacket* packet);

        /**
         * If this host has received a RERR packet, trim all routes from this 
         * host to where the packet found the broken link and onword
         */
        void handleRERR(DSRPacket* packet);

};

#endif