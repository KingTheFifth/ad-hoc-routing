#include "host.h"
#include "packet/DSDVPacket.h"

struct DSDVHost : public Host {
    public:
        DSDVHost(double _x, double _y, int _radius, int _time, unsigned _id)
            : Host(_x, _y, _radius, _time, _id) {}
        
        ~DSDVHost() = default;

    protected:
        void processPacket(Packet* packet);
    private:
        struct DSDVTable {
            Link* nextHop;
            Host* destination;
            unsigned entryVersion; // of the relevant host we got the update from
            unsigned cost; // amount of hops
        };
        unsigned tableVersion = 0; // TODO: set init value at a better time?
        vector<DSDVTable> routes;

        /**
         * 
         */
        Link* DSDV(DSDVPacket* packet);
};