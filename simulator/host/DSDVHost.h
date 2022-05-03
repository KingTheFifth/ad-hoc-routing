#ifndef DSDVHOST_H
#define DSDVHOST_H

#include "host.h"
#include "packet/DSDVPacket.h"
#include <utility>

struct DSDVHost : public Host {
    public:
        DSDVHost(double _x, double _y, int _radius, int _time, unsigned _id)
            : Host(_x, _y, _radius, _time, _id) {}
        
        ~DSDVHost() = default;

    protected:
        void processPacket(Packet* packet);
    private:
        struct DSDVTable {
            Link* nextHop;
            Host* destination;
            pair<unsigned, unsigned> sequenceNumber; // first is hostID, second is sequenceNumber
            unsigned cost; // amount of hops
        };
        unsigned sequenceNumber = 0; // TODO: set init value at a better time?
        vector<DSDVTable> routes;

        /**
         * 
         */
        Link* DSDV(DSDVPacket* packet);
};

#endif