#ifndef GPSRPACKET_H
#define GPSRPACKET_H

#include "packet.h"

class GPSRPacket : public Packet {
    public:
        GPSRPacket(const Host* _source, const Host* _destination, int _time, int _id = -1);
        ~GPSRPacket() = default;

        const Point* destPos; // D
        Point* prevPos; // Position of the host previously visited by the packet
        Point* failurePos; // L_p
        Point* destLineIntersect; // L_f
        pair<Host*, Host*> firstEdgeInPerim; // e_0, stores the first link in current perimeter, with a direction
        enum GPSRMode {Greedy, Perimeter};
        GPSRMode mode = Greedy;

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
