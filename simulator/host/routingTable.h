#ifndef ROUTINGTABLE_H
#define ROUTINGTABLE_H

#include <vector>
#include <utility>

using namespace std;

class DSDVHost;

struct Row {
    Row(DSDVHost* _destination, DSDVHost* _nextHop, double _cost, pair<DSDVHost*, unsigned> _sequenceNumber)
    : destination(_destination), nextHop(_nextHop), cost(_cost), sequenceNumber(_sequenceNumber){}

    Row(const Row* that) {
        destination = that->destination;
        nextHop = that->nextHop;
        cost = that->cost;
        sequenceNumber = that->sequenceNumber;
        hasChanged = that->hasChanged;
    }
    
    DSDVHost* destination;
    DSDVHost* nextHop; // Next host to traverse on route to destination.
    double cost; // Cost in geographical (link) distance traversed by a route
    pair<DSDVHost*, unsigned> sequenceNumber; //Sequence number to solve loops and count-to-infinity problem
    bool hasChanged = true; //Whether or not route has changed recently
    bool brokenRouteDetected = false; //Whether a broken route has been detected
};

class RoutingTable {
    public:
        vector<Row*> entries;
        bool brokenLinks = false;

        /**
         * Get the next host to move to when trying to get to 'destination'
         */
        DSDVHost* getNextHop(const DSDVHost* destination);

        /**
         * Insert an entry into the routing table of this host
         */
        void insert(DSDVHost* destination, DSDVHost* nextHop, double cost, pair<DSDVHost*, unsigned> sequenceNumber);

        /**
         * Remove an entry from the routing table of this host
         */
        void remove(DSDVHost* destination);

        /**
         * Updates local routing table with information from another table
         */
        void update(RoutingTable* otherTable);

        /**
         * Update the cost to the destination at row 'row' in the routing table
         */
        void updateCost(Row* row, double cost);

        /**
         * Get the cost of the route from this host to ither 'host' from routing table
         */
        int getCost(const DSDVHost* host);

        /**
         * Set the route as broken between this host and 'destination'
         */
        void setRouteBroken(DSDVHost* destination);

        /**
         * Get a RoutingTable containing (only) all changes done to the routing table since last call
         */
        RoutingTable* getChanges();

        /**
         * Get the number of changes done since last call of getChanges()
         */
        int getNumberOfChanges();

        /**
         * Get all entries in this routing table
         */
        vector<Row*>* getEntries();

    private:

        /**
         * Get a specific row entry of the Host 'host'
         */
        Row* getEntry(const DSDVHost* host);
};


#endif // ROUTINGTABLE_H
