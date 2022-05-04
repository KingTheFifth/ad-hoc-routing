#ifndef ROUTINGTABLE_H
#define ROUTINGTABLE_H

#include <vector>
#include <utility>

using namespace std;

class DSDVHost;

struct Row {
    Row(DSDVHost* _destination, DSDVHost* _nextHop, double _cost, pair<DSDVHost*, unsigned> _sequenceNumber)
    : destination(_destination), nextHop(_nextHop), cost(_cost), sequenceNumber(_sequenceNumber){}
    Row(Row* otherRow);
    DSDVHost* destination;
    DSDVHost* nextHop; //For DSDV the route is just a single node (next-hop)
    double cost; //Cost in geographical (link) distance traversed by a route
    pair<DSDVHost*, unsigned> sequenceNumber;
    bool hasChanged = true;
};

class RoutingTable{
    public:
        RoutingTable();
        DSDVHost* getNextHop(DSDVHost* destination);
        void insert(DSDVHost* destination, DSDVHost* route, double cost, pair<DSDVHost*, unsigned> sequenceNumber);
        void remove(DSDVHost* destination);
        void update(RoutingTable* otherTable); //updates local routing table from other table
        void updateCost(Row* row, double cost);
        RoutingTable* getChanges();
        vector<Row*>* getEntries();
    private:
        Row* getEntry(DSDVHost* host);
        vector<Row*> entries;
};


#endif // ROUTINGTABLE_H