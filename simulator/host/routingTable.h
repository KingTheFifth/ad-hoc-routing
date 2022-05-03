#ifndef ROUTINGTABLE_H
#define ROUTINGTABLE_H

#include <vector>
#include <utility>

using namespace std;

class Host;

struct Row {
    Row(Host* _destination, Host* _nextHop, double _cost, pair<Host*, unsigned> _sequenceNumber)
    : destination(_destination), nextHop(_nextHop), cost(_cost), sequenceNumber(_sequenceNumber){}
    Host* destination;
    Host* nextHop; //For DSDV the route is just a single node (next-hop)
    double cost; //Cost in geographical (link) distance traversed by a route
    pair<Host*, unsigned> sequenceNumber;
    bool hasChanged = true;
};

class RoutingTable{
    public:
        RoutingTable();
        vector <Host*>* getRoute(Host* destination);
        void insert(Host* destination, Host* route, double cost, pair<Host*, unsigned> sequenceNumber);
        void remove(Host* destination);
        void update(RoutingTable* otherTable); //updates local routing table from other table
        void updateCost(Row* row, double cost);
        RoutingTable* getChanges();
        vector<Row*>* getEntries();
    private:
        Row* getEntry(Host* host);
        vector<Row*> entries;
};


#endif // ROUTINGTABLE_H
