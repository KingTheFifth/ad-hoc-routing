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
         * 
         */
        void broadcastTable(RoutingTable* table);

    protected:
        /**
         * 
         */
        void processPacket(Packet* packet);

    private:
        int nextBroadcast = 0;
        int nextFullBroadcast = 0;
        bool awaitingBroadcast = true;

        /**
         * 
         */
        void broadcastChanges();

        /**
         * 
         */
        bool shouldBroadcast(int currTime);

        /**
         * 
         */
        void tick(int currTime);

        /**
         * 
         */
        Link* DSDV(DSDVPacket* packet);

        /**
         * 
         */
        void dropReceivedPacket(Packet* packet);

        /**
         *
         */
        void countPacketDrop(Packet* packet);

        void deleteRoutes(Host* destination);
};

#endif
