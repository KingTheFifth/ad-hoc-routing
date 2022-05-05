#include "DSDVHost.h"
#include "link.h"
#include "routingTable.h"
#include "packet/DSDVPacket.h"
#include <iostream>

DSDVHost::DSDVHost(StatisticsHandler* _statistics, double _x, double _y, int _radius, int _time, unsigned _id)
            : Host(_statistics, _x, _y, _radius, _time, _id) {
                routingTable = new RoutingTable();
                routingTable->insert(this, this, 0.0, make_pair(this, (unsigned) 0));
                }

void DSDVHost::processPacket(Packet* packet) {
    DSDVPacket* dsdvPacket = (DSDVPacket*) packet;
    DSDVPacket::PacketType type = dsdvPacket->packetType;
    if (type == DSDVPacket::BROADCAST){ //We received a broadcasted routing table. Update ours
        routingTable->update(dsdvPacket->routingTable);
        RoutingTable* ourChanges = routingTable->getChanges();
        if (ourChanges->entries.size() > 1){
            broadcastTable(ourChanges);
        }
    }
    else { //Normal data packet.
        const DSDVHost* dest = (DSDVHost*)(dsdvPacket->destination);
        DSDVHost* nextHop = routingTable->getNextHop(dest);
        if(nextHop != nullptr){ //Destination found in table
            Link* link = getLinkToHost((Host*)nextHop);
            transmitBuffer.push(make_pair(dsdvPacket, link));
        }
        //DSDV does not handle cases where no destination is found, since all hosts 'should' be familiar. Drop the packet.
    }
}

void DSDVHost::broadcastTable(RoutingTable* table) {
    DSDVPacket* broadcast = new DSDVPacket(this, nullptr, time); //create packet of BROADCAST type with pointer to table
    broadcast->packetType = DSDVPacket::BROADCAST;
    broadcast->routingTable = table;
    broadcast->source = this;
    for(vector<Link*>::iterator neighbour = neighbours.begin(); neighbour != neighbours.end(); neighbour++){
        forwardPacket(broadcast->copy(), *neighbour);
    }
}
