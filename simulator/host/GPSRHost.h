#include "host.h"
#include "packet/GPSRPacket.h"

struct GPSRHost : public Host {
    public:
        GPSRHost(double _x, double _y, int _radius, int _time, unsigned _id)
            : Host(_x, _y, _radius, _time, _id) {}

        ~GPSRHost() = default;

    protected:
        void processPacket(Packet* packet) override;
    private:
        vector<Link*> perimeterLinks;

        /**
         * 
         */
        Link* GPSR(GPSRPacket* packet);

        /**
         * 
         */
        void getPerimeterLinks(vector<Link*>* result);

        /**
         * 
         */
        Link* getRHREdge(const Point* referencePoint, vector<Link*>* perimeterLinks) const;
};