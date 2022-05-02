#ifndef PACKET_H
#define PACKET_H

#include <QGraphicsScene>
#include "constants.h"
#include "point.h"
//#include "host.h"

using namespace std;

class Host;
class Link;

class Packet {
    public:
        Packet(const Host* _source, const Host* _destination);

        // Copy constructor
        Packet(const Packet& other);
        
        const Host* source;
        const Host* destination;
        Host* nextHop;
        int size = PACKET_SIZE;

        // DSR specific
        // :)

        // DSDV specific
        // :)
        
        // GPSR specific
        // const Point* destPos; // D
        // Point* prevPos; // Position of the host previously visited by the packet
        // Point* failurePos; // L_p
        // Point* destLineIntersect; // L_f
        // Link* firstEdgeInPerim; // e_0, stores the first link in current perimeter
        // enum GPSRMode {Greedy, Perimeter};
        // GPSRMode mode = Greedy;

    private:
        
};

#endif