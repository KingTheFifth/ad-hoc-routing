#ifndef LINK_H
#define LINK_H

#include <utility>
//#include "host.h"
#include <QGraphicsScene>
#include "packet/packet.h"
#include <vector>

using namespace std;

class Host;

struct PacketOnLink {
    Packet* packet;
    int timeOnLink;
    Point* origin;
};

class Link {
    public:
        Link(Host *hostA, Host *hostB, int currTime);
        /**
         * Draws this link as a line between the two hosts of the link.
         */
        void draw(QGraphicsScene *scene);
        /**
         * Draws this link as a line between the two hosts of the link, with special styling
         */
        void drawAsPerimeter(QGraphicsScene *scene);

        /**
         * 
         */
        Host* getOtherHost(const Host *currentHost);

        /**
         * Puts given packet into the link's buffer while in transit
         */
        void forwardPacket(Packet *packet);

        /**
         * 
         */
        void tick(int currTime);

    private:
        pair<Host*, Host*> hosts;
        //int length; // To calculate a time delay over the link
        int time;

        vector<PacketOnLink*> linkBuffer;
};

#endif