#include "DSDVHost.h"
#include "link.h"
#include "routingTable.h"
#include "packet/DSDVPacket.h"
#include <iostream>

DSDVHost::DSDVHost(StatisticsHandler* _statistics, double _x, double _y, int _radius, int _time, unsigned _id, unordered_map<unsigned, Host*>* _hosts)
            : Host(_statistics, _x, _y, _radius, _time, _id, _hosts) {
                routingTable = new RoutingTable();
                routingTable->insert(this, this, 0.0, make_pair(this, (unsigned) 0));
                }

DSDVHost::~DSDVHost() {
    while (!transmitBuffer.empty()) {
        Packet* p = transmitBuffer.front().first;
        transmitBuffer.pop();
        dropReceivedPacket(p);
    }

    while (!receivingBuffer.empty()) {
        Packet* p = receivingBuffer.front();
        receivingBuffer.pop();
        dropReceivedPacket(p);
    }
}

void DSDVHost::processPacket(Packet* packet) {
    DSDVPacket* dsdvPacket = (DSDVPacket*) packet;
    DSDVPacket::PacketType type = dsdvPacket->packetType;
    if (type == DSDVPacket::BROADCAST) { // We received a broadcasted routing table. Update ours
        routingTable->update(dsdvPacket->routingTable);
        int numberOfChanges = routingTable->getNumberOfChanges();
        if(routingTable->brokenLinks){
            awaitingBroadcast = true;
            routingTable->brokenLinks = false;
        }
        if (routingTable->entries[0]->hasChanged == true || numberOfChanges > 1){
            awaitingBroadcast = true;
        }
    }
    else { // Normal data packet.
        const DSDVHost* dest = (DSDVHost*)(dsdvPacket->destination);
        if (dest == this) { // If this is the intended destination
            int delay = time - dsdvPacket->timeSent;
            statistics->addPacketArrival(delay); // Count packet as arrived
            delete dsdvPacket;
            return;
        }
        DSDVHost* nextHop = routingTable->getNextHop(dest);
        int nextHopCost = routingTable->getCost(nextHop);
        if(nextHop && nextHopCost != std::numeric_limits<int>::infinity()){ // If the destination is found in this hosts table
            Link* link = getLinkToHost(nextHop);
            if (link) { // If link is not broken
                forwardPacket(dsdvPacket, link);
            }
            else {
                dropReceivedPacket(packet);
            }
        }
        else { // DSDV does not handle cases where no destination is found, since all hosts 'should' be familiar
            dropReceivedPacket(packet); // Drop the packet
        }
    }
}

void DSDVHost::broadcastTable(RoutingTable* table) {
    DSDVPacket* broadcastPacket = new DSDVPacket(this, nullptr, time); // Create packet of BROADCAST type with pointer to table
    broadcastPacket->packetType = DSDVPacket::BROADCAST;
    broadcastPacket->color = Qt::red;
    broadcastPacket->routingTable = table;
    broadcastPacket->source = this;

    // Broadcast to all neighbours
    broadcast(broadcastPacket);
    statistics->addRoutingPackets(neighbours.size());
}

void DSDVHost::broadcastChanges(){
    RoutingTable* ourChanges = routingTable->getChanges();
    broadcastTable(ourChanges);
}


bool DSDVHost::shouldBroadcast(int currTime){
    bool partialBroadcast = currTime > nextBroadcast && awaitingBroadcast;
    bool fullBroadcast = currTime > nextFullBroadcast;
    return partialBroadcast || fullBroadcast;
}


void DSDVHost::tick(int currTime){
    Host::tick(currTime);
    
    if(shouldBroadcast(currTime)){
        if(currTime > nextFullBroadcast) {
            routingTable->entries[0]->sequenceNumber.second += 2;
            broadcastTable(routingTable);
            nextFullBroadcast = currTime + FULLBROADCASTDELAY;
        }
        else {
            broadcastChanges();
        }
        nextBroadcast = currTime + BROADCASTDELAY;
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
    awaitingBroadcast = true; // Routes have broken. Broadcast ASAP
}
