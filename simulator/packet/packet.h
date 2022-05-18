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
        Packet(const Host* _source, const Host* _destination, int _time, int _id = -1);

        // Copy constructor
        Packet(const Packet& other);

        virtual ~Packet() = default;
        
        const Host* source;
        const Host* destination;
        Host* nextHop;
        int size = PACKET_SIZE;
        int timeSent;
        int id;
        QColor color = Qt::green;

        /**
         * Copy 'other' and set other's values to this packet's values
         */
        virtual void copyOther(const Packet& other);

        /** 
         * Make a copy of this packet
         */
        virtual Packet* copy() = 0;

        // virtual void draw(QGraphicsScene *scene, int x, int y) const {}
        

    private:

        
};

    void operator delete(void * p);
#endif
