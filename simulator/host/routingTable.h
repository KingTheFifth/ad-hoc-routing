#ifndef ROUTINGTABLE_H
#define ROUTINGTABLE_H

#include <vector>

using namespace std;

class Host;

struct Row {
    Host* destination;
    vector<Host*> route; //For DSDV the route is just a single node (next-hop)
    int cost;
    int sequenceNumber;
    int installTime;
};

class RoutingTable{
    public:
        vector <Host*>* getRoute(Host* destination);
        void insert(Host* destination, vector<Host*> route, int cost = 0, int sequenceNumber = 0, int installTime = 0);
        void insert(Host* destination, Host* route, int cost = 0, int sequenceNumber = 0, int installTime = 0);
        void remove(Host* destination);
        void update(RoutingTable* otherTable); //updates local routing table from other table
    private:
        Row* getEntry(Host* host);
        vector<Row*> entries;
};


#endif // ROUTINGTABLE_H
