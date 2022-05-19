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
         *
         */
        DSDVHost* getNextHop(const DSDVHost* destination);

        /**
         *
         */
        void insert(DSDVHost* destination, DSDVHost* nextHop, double cost, pair<DSDVHost*, unsigned> sequenceNumber);

        /**
         *
         */
        void remove(DSDVHost* destination);

        /**
         * Updates local routing table from other table
         */
        void update(RoutingTable* otherTable);

        /**
         *
         */
        void updateCost(Row* row, double cost);

        /**
         *
         */
        int getCost(const DSDVHost* host);

        /**
         *
         */
        void setRouteBroken(DSDVHost* destination);

        /**
         *
         */
        RoutingTable* getChanges();

        /**
         *
         */
        int getNumberOfChanges();

        /**
         *
         */
        vector<Row*>* getEntries();

    private:

        /**
         *
         */
        Row* getEntry(const DSDVHost* host);
};


#endif // ROUTINGTABLE_H
