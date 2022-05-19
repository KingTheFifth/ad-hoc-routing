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
    DSDVHost* nextHop; // For DSDV the route is just a single node (next-hop)
    double cost; // Cost in geographical (link) distance traversed by a route
    pair<DSDVHost*, unsigned> sequenceNumber;
    bool hasChanged = true;
    bool brokenRouteDetected = false;
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
         * Updates local routing table from other table
         */
        void update(RoutingTable* otherTable);

        /**
         * Update the cost to the destination at row 'row' in the routing table
         */
        void updateCost(Row* row, double cost);

        /**
         * Get the cost from this host to 'host' out of the routing table
         */
        int getCost(const DSDVHost* host);

        /**
         * Set the route as broken between this host and 'destination'
         */
        void setRouteBroken(DSDVHost* destination);

        /**
         * Get a RoutingTable with all changes done to the routing table since last call
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
         * Get a specific entry of the Host 'host'
         */
        Row* getEntry(const DSDVHost* host);
};


#endif // ROUTINGTABLE_H
