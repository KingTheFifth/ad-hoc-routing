#ifndef PACKET_H
#define PACKET_H

#include <QGraphicsScene>
#include "constants.h"
#include "point.h"
//#include "host.h"

using namespace std;

class Host;

struct Packet {
    public:
        Packet(const Host* _source, const Host* _destination);
        const Host* source;
        const Host* destination;
        Host* nextHop;
        int size = PACKET_SIZE;
        
        // GPSR specific
        const Point* destPos; // D
        Point* prevPos; // Position of the host previously visited by the packet
        Point* failurePos; // L_p
        Point* destLineIntersect; // L_f
        Host* firstEdgeInPerim; // e_0, stores the first link in current perimeter as the "second" host of the link

        enum GPSRMode {Greedy, Perimeter};
        GPSRMode mode = Greedy;

    private:
        
};

#endif