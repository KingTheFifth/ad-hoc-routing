#ifndef GPSRPACKET_H
#define GPSRPACKET_H

#include "packet.h"

class GPSRPacket : public Packet {
    public:
        GPSRPacket(const Host* _source, const Host* _destination, int _time);
        ~GPSRPacket() = default;

        const Point* destPos; // D, the destination of the packet
        Point* prevPos; // Position of the host previously visited by the packet
        Point* failurePos; // L_p, the position at where Greedy Forwarding failed
        Point* destLineIntersect; // L_f, the intersection point of where the line from
        // L_p to D and a link in the current planarised graph crosses each other
        pair<Host*, Host*> firstEdgeInPerim; // e_0, stores the first link in current perimeter, with a direction
        enum GPSRMode {Greedy, Perimeter}; // The different modes a GPSRPacket can have. This determines what routing algorithm to use
        GPSRMode mode = Greedy; // Initially all packets are using Greedy Forwarding

        /**
         * Copy 'other' and set other's values to this packet's values
         */
        void copyOther(const Packet& other) override {}

        /** 
         * Make a copy of this packet
         */
        Packet* copy() override {}
};

#endif
