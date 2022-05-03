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
        Packet() {}
        Packet(const Host* _source, const Host* _destination);

        // Copy constructor
        Packet(const Packet& other);

        virtual ~Packet() = default;
        
        const Host* source;
        const Host* destination;
        Host* nextHop;
        int size = PACKET_SIZE;

        virtual void copyOther(const Packet& other);
        virtual Packet* copy() = 0;

    private:
        
};

#endif