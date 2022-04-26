#ifndef PACKET_H
#define PACKET_H

#include <QGraphicsScene>
#include "constants.h"
//#include "host.h"


using namespace std;

class Host;

struct Packet {
    public:
        Packet(const Host* _source, const Host* _destination)
        : source(_source), destination(_destination) {}
        const Host* source;
        const Host* destination;
        Host* nextHop;
        int size = PACKET_SIZE;

    private:
        
};

#endif