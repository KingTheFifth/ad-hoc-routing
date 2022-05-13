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
    if (type == DSDVPacket::BROADCAST) { //We received a broadcasted routing table. Update ours
        routingTable->update(dsdvPacket->routingTable);
        int numberOfChanges = routingTable->getNumberOfChanges();
        //TODO: Check if brokenRouteDetected flag in routing table is true.
        //If so, BYPASS BROADCAST DELAY!!! Info needs to go out asap.
        //Change currTime >= lastBroadcast + delay to currTime >= nextBroadcast (calculated as currTime + delay when we set it)
        //TODO: Set brokenRouteDetected to false as soon as we broadcast that info?
        if (routingTable->entries[0]->hasChanged == true || numberOfChanges > 1){
            awaitingBroadcast = true;
        }
    }
    else { //Normal data packet.
        const DSDVHost* dest = (DSDVHost*)(dsdvPacket->destination);
        if (dest == this) {
            int delay = time - dsdvPacket->timeSent;
            statistics->addPacketArrival(delay);
            delete dsdvPacket;
            return;
        }

        DSDVHost* nextHop = routingTable->getNextHop(dest);
        if(nextHop != nullptr){ //Destination found in table
            Link* link = getLinkToHost(nextHop);
            forwardPacket(dsdvPacket, link);
        }
        else {
            //DSDV does not handle cases where no destination is found, since all hosts 'should' be familiar. Drop the packet.
            dropReceivedPacket(packet);
        }
    }
}

void DSDVHost::broadcastTable(RoutingTable* table) {
    DSDVPacket* broadcastPacket = new DSDVPacket(this, nullptr, time); //create packet of BROADCAST type with pointer to table
    broadcastPacket->packetType = DSDVPacket::BROADCAST;
    broadcastPacket->routingTable = table;
    broadcastPacket->source = this;
    broadcast(broadcastPacket);
    statistics->addRoutingPackets(neighbours.size());
}

void DSDVHost::broadcastChanges(){
    RoutingTable* ourChanges = routingTable->getChanges();
    broadcastTable(ourChanges);
}


bool DSDVHost::shouldBroadcast(int currTime){
    bool partialBroadcast = currTime - lastBroadcast > BROADCASTDELAY && awaitingBroadcast;
    bool fullBroadcast = currTime - lastFullBroadcast > FULLBROADCASTDELAY;
    return partialBroadcast || fullBroadcast;
}


void DSDVHost::tick(int currTime){
    Host::tick(currTime);
    if(shouldBroadcast(currTime)){
        if(currTime - lastFullBroadcast > FULLBROADCASTDELAY) {
            routingTable->entries[0]->sequenceNumber.second += 2;
            broadcastTable(routingTable);
            lastFullBroadcast = currTime;
        }
        else {
            broadcastChanges();
        }
        lastBroadcast = currTime;
        awaitingBroadcast = false;
    }
}

void DSDVHost::dropReceivedPacket(Packet* packet) {
    countPacketDrop(packet);
    delete packet;
}

void DSDVHost::countPacketDrop(Packet* packet) {
    DSDVPacket* dsdvPacket = dynamic_cast<DSDVPacket*>(packet);
    if (dsdvPacket->packetType == DSDVPacket::PacketType::OTHER) {
        statistics->dropDataPacket();
    }
}

void DSDVHost::deleteRoutes(Host* destination) {
    DSDVHost* dsdvHost = dynamic_cast<DSDVHost*>(destination);
    routingTable->setRouteBroken(dsdvHost);
    awaitingBroadcast = true; //Routes have broken. Broadcast ASAP
}
