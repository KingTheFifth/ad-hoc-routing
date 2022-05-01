#include "host.h"
#include <vector>

struct RREQ {
    Host* initiator;
    Host* target;
    int requestID;
    vector<Host*> route; // Is there something better than vector to use?
}

struct RREP {
    int requestID;
    vector<Host*> route; // Is there something better than vector to use?
}