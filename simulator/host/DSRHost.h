#include "host.h"
#include "packet/DSRPacket.h"

struct DSRHost : public Host {
    public:
        DSRHost(double _x, double _y, int _radius, int _time, unsigned _id)
            : Host(_x, _y, _radius, _time, _id) {}
        // void tick(int currTime);

        ~DSRHost() = default;

    protected:
        void processPacket(Packet* packet);
    private:
        struct DSRRoute {
            vector<Host*> route;

            // Adds a node to the end of the route
            void addNode(Host* node) {

            }

            // Returns true if the given host is somehow reachable through this route
            bool hasTarget(const Host* target) {
                return false;
            }

            // Returns the next hop host from the given source host according to this route.
            // Works under the assumption that a route does not necessarily start with the 
            // node that is forwarding a packet along a route.
            Host* getNextHop(const Host* source) {
                return nullptr;
            }
        };
        vector<DSRRoute*> routes;

        /**
         * 
         */
        Link* DSR(DSRPacket* packet);
};