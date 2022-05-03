#include "routingTable.h"

vector<Host*>* RoutingTable::getRoute(Host *destination){
    struct Row* tableEntry = getEntry(destination);
    if (tableEntry == nullptr) { return nullptr; }
    return &(tableEntry->route);
}

void RoutingTable::insert(Host* destination, vector<Host*> route, int cost, int sequenceNumber, int installTime){
    //Create new row
}

void RoutingTable::insert(Host* destination, Host* route, int cost, int sequenceNumber, int installTime){
    //Create route vector and call other insert function
}

void RoutingTable::remove(Host* destination){
    for (vector<Row*>::iterator it = entries.begin(); it != entries.end(); it++){
        if((*it)->destination == destination){
            entries.erase(it);
            break;
        }
    }
}

void RoutingTable::update(RoutingTable* otherTable){
    return;
}

Row* RoutingTable::getEntry(Host* host){
    Row* entry = nullptr;
    for (vector<Row*>::iterator it = entries.begin(); it != entries.end(); it++){
        if((*it)->destination == host){
            entry = *it;
            break;
        }
    }
    return entry;
}
