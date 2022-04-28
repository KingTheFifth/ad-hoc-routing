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
        const Point* destPos; // TODO:change?
        Point* prevPos; // is this the best way to do this?
        enum GPSRMode {Greedy, Perimeter};
        GPSRMode mode = Greedy;

    private:
        
};

#endif