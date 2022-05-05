#include "DSDVHost.h"
#include "link.h"
#include "routingTable.h"

DSDVHost::DSDVHost(StatisticsHandler* _statistics, double _x, double _y, int _radius, int _time, unsigned _id)
            : Host(_statistics, _x, _y, _radius, _time, _id) {
                routingTable = new RoutingTable();
                routingTable->insert(this, this, 0.0, make_pair(this, (unsigned) 0));
                }

Link* DSDVHost::DSDV(DSDVPacket* packet) {
    // Do DSDV
}

void DSDVHost::processPacket(Packet* packet) {
    //Check if packet contains a routingTable, if so run updateTable
    //broadcast updates

    //If not a routingTable packet, lookup nexthop and forward.
}

void DSDVHost::broadcastTable(RoutingTable* table){
    DSDVPacket* broadcast = new DSDVPacket(); //create packet of BROADCAST type with pointer to table
    for(vector<Link*>::iterator neighbour = neighbours.begin(); neighbour != neighbours.end(); neighbour++){

    }
}