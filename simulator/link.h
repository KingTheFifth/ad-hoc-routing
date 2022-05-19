#ifndef LINK_H
#define LINK_H

#include <utility>
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
        bool isBroken;

        Link(Host *hostA, Host *hostB, int currTime);

        ~Link();

        /**
         * Draws this link as a line between the two hosts of the link.
         */
        void draw(QGraphicsScene *scene);
        /**
         * Draws this link as a line between the two hosts of the link, with special styling
         */
        void drawAsPerimeter(QGraphicsScene *scene);

        /**
         * Get the opposing host of 'currentHost' on this link
         */
        Host* getOtherHost(const Host *currentHost);

        /**
         * Get the length of this link
         */
        double getLength();

        /**
         *
         */
        void getPackets(vector<Packet*>* resultVector);

        /**
         * Puts given packet into the link's buffer while in transit
         */
        void forwardPacket(Packet *packet);

        /**
         * Tick the simulation forward one step on this link
         */
        void tick(int currTime);

        pair<Host*, Host*> hosts;
    private:
        int time;

        vector<PacketOnLink*> linkBuffer;
};

#endif
