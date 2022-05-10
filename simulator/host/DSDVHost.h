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
        DSDVHost(StatisticsHandler* _statistics, double _x, double _y, int _radius, int _time, unsigned _id);
        
        ~DSDVHost() = default;

        RoutingTable* routingTable;
        void broadcastTable(RoutingTable* table);
    protected:
        void processPacket(Packet* packet);
    private:
        int lastBroadcast = -BROADCASTDELAY;
        int lastFullBroadcast = -FULLBROADCASTDELAY;
        bool awaitingBroadcast = true;
        void broadcastChanges();
        bool shouldBroadcast(int currTime);
        void tick(int currTime);


        /**
         * 
         */
        Link* DSDV(DSDVPacket* packet);
};

#endif
