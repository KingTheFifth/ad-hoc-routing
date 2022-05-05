#include "routingTable.h"
#include "DSDVHost.h"

//TODO: Implement sequence numbers. Look up how sequence numbers work first.
//Specifically who discovers and broadcasts a broken link and with what sequence number.

DSDVHost* RoutingTable::getNextHop(const DSDVHost *destination){
    struct Row* tableEntry = getEntry(destination);
    if (tableEntry == nullptr) { return nullptr; }
    return tableEntry->nextHop;
}

void RoutingTable::insert(DSDVHost* destination, DSDVHost* nextHop, double cost, pair<DSDVHost*, unsigned> sequenceNumber) {
    Row* row = new Row(destination, nextHop, cost, sequenceNumber);
    entries.push_back(row);
}

void RoutingTable::remove(DSDVHost* destination){
    for (vector<Row*>::iterator it = entries.begin(); it != entries.end(); it++){
        if((*it)->destination == destination){
            entries.erase(it);
            break;
        }
    }
}

void RoutingTable::update(RoutingTable* otherTable){
    //Every time we get a routing update, we should check if our distance to that neighbour needs updating.
    Row* neighbour = getEntry(otherTable->getEntries()->front()->destination); //Check if we have neighbour row in our own table, extracting host* from neighbour's table.
    if (neighbour == nullptr){
        DSDVHost* thisHost = entries[0]->destination;
        DSDVHost* neighbourHost = neighbour->destination;
        insert(neighbourHost, neighbourHost, thisHost->distanceTo(neighbourHost), neighbour->sequenceNumber);
    }
    else { //if neighbour not in table, add it.
        double newCost = entries[0]->destination->distanceTo(otherTable->entries.front()->destination);
        if (newCost < neighbour->cost){
            neighbour->cost = newCost;
            neighbour->hasChanged = true;
        }
    }
    vector<Row*>::iterator otherEntry = otherTable->getEntries()->begin(); //Iterate through every entry in received table
    if(otherEntry != otherTable->getEntries()->end()){ otherEntry++; } //We want to skip neighbour. If list was empty, we need the if-statement to stay within bounds.
    while(otherEntry != otherTable->getEntries()->end()){
        for(vector<Row*>::iterator ourEntry = entries.begin(); ourEntry != entries.end(); ourEntry++){
            if((*otherEntry)->destination == (*ourEntry)->destination){
                double routeCost = entries[0]->destination->distanceTo(otherTable->entries.front()->destination) + (*otherEntry)->cost;
                if (routeCost < (*ourEntry)->cost){
                    remove((*ourEntry)->destination); //remove our row, then create a new one.
                    insert((*otherEntry)->destination, otherTable->entries.front()->destination, routeCost, (*otherEntry)->sequenceNumber);
                }
                break; //match already found. Continue to next entry
            }
        }
        double routeCost = entries[0]->destination->distanceTo(otherTable->entries.front()->destination) + (*otherEntry)->cost;
        insert((*otherEntry)->destination, otherTable->entries.front()->destination, routeCost, (*otherEntry)->sequenceNumber);
    }
}

void RoutingTable::updateCost(Row* row, double cost){
    row->cost = cost;
    row->hasChanged = true;
}

RoutingTable* RoutingTable::getChanges(){
    RoutingTable* tableChanges = new RoutingTable();
    for (vector<Row*>::iterator entry = entries.begin(); entry != entries.end(); entry++){
        if((*entry)->hasChanged){
            Row* newEntry = new Row(*entry);
            tableChanges->entries.push_back(newEntry);
        }
    }
    //TODO: Make sure to set row.hasChanged to false after we broadcast a route.
    //TODO: create new table from all changed entries.
}

Row* RoutingTable::getEntry(const DSDVHost* host){
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
