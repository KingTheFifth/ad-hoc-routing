#ifndef LINK_H
#define LINK_H

#include <utility>
//#include "host.h"
#include <QGraphicsScene>

using namespace std;

class Host;

class Link {
    public:
        Link(Host *hostA, Host *hostB);
        /**
         * Draws this link as a line between the two hosts of the link.
         */
        void draw(QGraphicsScene *scene);

    private:
        bool isDrawn = false;
        pair<Host*, Host*> hosts;
        int length; // To calculate a time delay over the link
};

#endif