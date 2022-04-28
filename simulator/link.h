#ifndef LINK_H
#define LINK_H

#include <utility>
//#include "host.h"
#include <QGraphicsScene>
#include "packet.h"
#include <vector>

using namespace std;

class Host;

class Link {
    public:
        Link(Host *hostA, Host *hostB, int currTime);
        /**
         * Draws this link as a line between the two hosts of the link.
         */
        void draw(QGraphicsScene *scene);

        Host* getOtherHost(Host *currentHost);

        /**
        * Puts given packet into the link's buffer while in transit
        */
        void forwardPacket(Packet *packet);

        void tick(int currTime);

    private:


        pair<Host*, Host*> hosts;
        int length; // To calculate a time delay over the link
        int time;

        vector<pair<Packet*, int>> linkBuffer;
};

#endif