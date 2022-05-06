#ifndef DSDVHOST_H
#define DSDVHOST_H

#include "host.h"
// #include "routingTable.h"
#include "packet/DSDVPacket.h"
#include <utility>
#define BROADCASTDELAY 500

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
        bool awaitingBroadcast = true;
        void broadcastChanges();
        bool ShouldBroadcast(int currTime);
        void tick(int currTime);


        /**
         * 
         */
        Link* DSDV(DSDVPacket* packet);
};

#endif
