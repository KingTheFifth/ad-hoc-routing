#include "routingTable.h"
#include "DSDVHost.h"
#include <iostream>

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
    
    DSDVHost* neighbourHost = otherTable->entries[0]->destination;
    DSDVHost* thisHost = entries[0]->destination;

    bool neighbourChanged = false;
    Row* neighbourEntry = getEntry(neighbourHost);

    if(!neighbourEntry) {
        insert(neighbourHost, neighbourHost, thisHost->distanceTo(neighbourHost), otherTable->entries[0]->sequenceNumber);
        neighbourChanged = true;
    }
    else {
        neighbourChanged = otherTable->entries[0]->sequenceNumber.second > neighbourEntry->sequenceNumber.second;
    }

    for(vector<Row*>::iterator otherEntry = otherTable->entries.begin(); otherEntry != otherTable->entries.end(); otherEntry++){
        bool entryFound = false;
        if((*otherEntry)->destination == this->entries[0]->destination && (*otherEntry)->sequenceNumber.second % 2 == 1){
            brokenLinks = true;
        }
        else{
            for(vector<Row*>::iterator ourEntry = entries.begin(); ourEntry != entries.end(); ourEntry++){
                if((*otherEntry)->destination == (*ourEntry)->destination){ //found a matching destination
                    entryFound = true;
                    if ((*ourEntry)->nextHop == neighbourHost && neighbourChanged){ //If neighbour has changed, this route through neighbour needs updating regardless
                        updateCost(*ourEntry, (*otherEntry)->cost + thisHost->distanceTo(neighbourHost)); //update cost of route this->neighbour->location
                        break; //Only matching entry found. proceed to next.
                    }
                    if((*otherEntry)->sequenceNumber.second > (*ourEntry)->sequenceNumber.second){ //Check if sequence number is newer (higher) than locally stored route
                        if ((*ourEntry)->cost == std::numeric_limits<int>::infinity() && (*otherEntry)->cost != std::numeric_limits<int>::infinity()){
                            brokenLinks = true;
                        }
                        ourEntry = entries.erase(ourEntry) - 1;
                        Row* newRow = new Row((*otherEntry)); //Copy their row
                        newRow->nextHop = neighbourHost; //Update nextHop to neighbour
                        newRow->cost += thisHost->distanceTo(neighbourHost); //Update proper cost (distance)

                        if((*otherEntry)->cost == std::numeric_limits<int>::infinity()){
                            newRow->cost = std::numeric_limits<int>::infinity();
                            brokenLinks = true;
                        }

                        entries.push_back(newRow);
                        break;
                    }
                    else if ((*otherEntry)->sequenceNumber.second == (*ourEntry)->sequenceNumber.second){ //Check if sequence number is same
                        if ((*ourEntry)->cost >= (*otherEntry)->cost + thisHost->distanceTo(neighbourHost)){ //Check if this route is cheaper than our locally stored
                            //cout << "other route is cheaper" << endl;
                            ourEntry = entries.erase(ourEntry) - 1;
                            Row* newRow = new Row((*otherEntry)); //Copy their row
                            newRow->nextHop = neighbourHost; //Update nextHop to neighbour
                            newRow->cost += thisHost->distanceTo(neighbourHost); //Update proper cost (distance)
                            entries.push_back(newRow);
                            break;
                        }
                    }
                    break; //match found and acton taken. Proceed to next entry.
                }
            }
            if (!entryFound) {
                insert((*otherEntry)->destination, neighbourHost, (*otherEntry)->cost + thisHost->distanceTo(neighbourHost), (*otherEntry)->sequenceNumber);
            }
        }
    }
}

void RoutingTable::setRouteBroken(DSDVHost* destination){
    for (vector<Row*>::iterator entry = entries.begin(); entry != entries.end(); entry++){
        if((*entry)->nextHop == destination){
            (*entry)->cost = std::numeric_limits<int>::infinity();
            (*entry)->sequenceNumber.second += 1;
            (*entry)->hasChanged = true;
            brokenLinks = true;
        }
    }
}

void RoutingTable::updateCost(Row* row, double cost){
    row->cost = cost;
    row->hasChanged = true;
}

int RoutingTable::getCost(const DSDVHost* destination){
    int cost = std::numeric_limits<int>::infinity();
    for (vector<Row*>::iterator it = entries.begin(); it != entries.end(); it++){
        if((*it)->destination == destination){
            cost = (*it)->cost;
        }
    }
    return cost;
}

RoutingTable* RoutingTable::getChanges(){
    RoutingTable* tableChanges = new RoutingTable();
    if(entries[0]->hasChanged == false){ //We have to add first element to table of changes, regardless of whether it has changed or not.
        Row* newEntry = new Row(entries[0]);
        tableChanges->entries.push_back(newEntry);
    }
    for (vector<Row*>::iterator entry = entries.begin(); entry != entries.end(); entry++){
        if((*entry)->hasChanged){
            Row* newEntry = new Row(*entry);
            tableChanges->entries.push_back(newEntry);
            (*entry)->hasChanged = false;
        }
    }
    return tableChanges;
    //TODO: create new table from all changed entries.
}

int RoutingTable::getNumberOfChanges(){
    int count = 0;
    for (vector<Row*>::iterator entry = entries.begin(); entry != entries.end(); entry++){
        if((*entry)->hasChanged){
            count +=1;
        }
    }
    return count;
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
