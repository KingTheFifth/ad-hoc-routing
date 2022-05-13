#ifndef GPSRHOST_H
#define GPSRHOST_H

#include "host.h"
#include "packet/GPSRPacket.h"

struct GPSRHost : public Host {
    public:
        GPSRHost(StatisticsHandler* _statistics, double _x, double _y, int _radius, int _time, unsigned _id)
            : Host(_statistics, _x, _y, _radius, _time, _id) {}

        ~GPSRHost() = default;

    protected:
        /**
         * 
         */
        void processPacket(Packet* packet) override;

    private:

        /**
         * Run the GPSR algorithm and return the link for the packet to route across
         */
        Link* GPSR(GPSRPacket* packet);

        /**
         * Get the perimeter links for this host according to the Relative Neighbourhood Graph algorithm
         */
        void getPerimeterLinks(vector<Link*>* result);

        /**
         * Return the link that is the Right Hand Rule edge
         */
        Link* getRHREdge(const Point* referencePoint, vector<Link*>* perimeterLinks) const;

        /**
         * Drop 'packet'
         */
        void dropReceivedPacket(Packet* packet);

        void countPacketDrop(Packet* packet);

        void deleteRoutes(Host* destination);
};

#endif