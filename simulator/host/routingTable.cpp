#include "routingTable.h"

vector<Host*>* RoutingTable::getRoute(Host *destination){
    struct Row* tableEntry = getEntry(destination);
    if (tableEntry == nullptr) { return nullptr; }
    return &(tableEntry->route);
}

void RoutingTable::insert(Host* destination, Host* route, double cost, pair<Host*, unsigned) sequenceNumber){
    Row* row = new Row(destination, route, cost, sequenceNumber);
    entries.push_back(row);
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
    //Every time we get a routing update, we should check if our distance to neighbour needs updating.
    Row* neighbour = getEntry(otherTable[0]->destination);
    if (neighbour == nullptr){
        insert(otherTable[0])
    }
    else {
        double newCost = entries[0]->destination->distanceTo(otherTable[0]->destination);
        if(newCost < neighbour->cost){
            updateCost(neighbour, newCost)
        }
    }
    vector<Row*>::iterator otherEntry = otherTable->getEntries()->begin();
    while(otherEntry != otherTable->getEntries()->end()){
        for(vector<Row*>::iterator ourEntry = entries.begin(); ourEntry != entries.end(); ourEntry++){
            if(otherEntry->destination == ourEntry->destination){
                double routeCost = entries[0]->destination->distanceTo(otherTable[0]->destination) + otherEntry->cost;
                if (routeCost <= ourEntry->cost){
                    insert(otherEntry->destination, otherTable[0]->destination, routeCost, otherEntry.sequenceNumber);
                }
                break;
            }
        }
        double routeCost = entries[0]->destination->distanceTo(otherTable[0]->destination) + otherEntry->cost;
        insert(otherEntry->destination, otherTable[0]->destination, routeCost, otherEntry.sequenceNumber);
    }
}

void RoutingTable::updateCost(Row* row, double cost){
    row->cost = cost;
    row->hasChanged = true;
}

Routingtable* RoutingTable::getChanges(){
    //TODO: Make sure to set row.hasChanged to false after we broadcast a route.
    //TODO: create new table from all changed entries.
    return nullptr;
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

vector<Row*>* RoutingTable::getEntries(){
    return &entries;
}