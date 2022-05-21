#ifndef DSDVHOST_H
#define DSDVHOST_H

#include "host.h"
// #include "routingTable.h"
#include "packet/DSDVPacket.h"
#include <utility>
#include "constants.h"

class RoutingTable;

class DSDVHost : public Host {
    public:
        DSDVHost(StatisticsHandler* _statistics, double _x, double _y, int _radius, int _time, unsigned _id, unordered_map<unsigned, Host*>* _hosts);
        
        ~DSDVHost();

        RoutingTable* routingTable;

        /**
         * Broadcasts a routing table to all neighbours. Can be a partial table for incremental updates.
         */
        void broadcastTable(RoutingTable* table);

    protected:
        /**
         * Processes incoming packets according to DSDV routing protocol.
         */
        void processPacket(Packet* packet);

    private:
        int nextBroadcast = 0;
        int nextFullBroadcast = 0;
        bool awaitingBroadcast = true;

        /**
         * Broadcasts all recently changed routing information to neighbours in an incremental update.
         */
        void broadcastChanges();

        /**
         * Checks whether hosts should broadcast or not.
         */
        bool shouldBroadcast(int currTime);

        /**
         * Tick the simulation forward once, forwarding packets if appropriate.
         */
        void tick(int currTime);

        /**
         * Drops a packet and deletes it.
         */
        void dropReceivedPacket(Packet* packet);

        /**
         * Counts dropped packets for statistics handler.
         */
        void countPacketDrop(Packet* packet);

        /**
         * Deletes a route from its routing table.
         */
        void deleteRoutes(Host* destination);

};

#endif
