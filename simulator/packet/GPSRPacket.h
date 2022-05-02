#include "packet.h"

class GPSRPacket : public Packet {
    public:
        GPSRPacket(const Host* _source, const Host* _destination);

        const Point* destPos; // D
        Point* prevPos; // Position of the host previously visited by the packet
        Point* failurePos; // L_p
        Point* destLineIntersect; // L_f
        Link* firstEdgeInPerim; // e_0, stores the first link in current perimeter
        enum GPSRMode {Greedy, Perimeter};
        GPSRMode mode = Greedy;
};