#include <QApplication>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <thread>
#include <chrono>
#include "host/host.h"
#include "host/GPSRHost.h"
#include "host/DSRHost.h"
#include "host/DSDVHost.h"
#include "host/routingTable.h"
#include "point.h"
#include "constants.h"
#include "StatisticsHandler.h"
#include "events/EventHandler.h"
#include "events/Event.h"

using namespace std;

enum Protocol {DSDV, DSR, GPSR};

// Handling of different event types
void handleSendEvent(Event* event, unordered_map<unsigned, Host*>* hosts, Protocol protocol, int time);
void handleMoveEvent(Event* event, unordered_map<unsigned, Host*>* hosts);
void handleJoinEvent(Event* event, unordered_map<unsigned, Host*>* hosts, Protocol protocol, StatisticsHandler* statistics, int radius, int time, unsigned id);
void handleDisconnectEvent(Event* event, unordered_map<unsigned, Host*>* hosts);

int main(int argc, char *argv[])
{
    // Open input file
    string eventsFilename = "ev_small_dense_A_move.txt"; // THIS IS WHERE YOU CHANGE TOPOLOGY/EVENT SETS AAAAAAAAAAAAAAAAAHHHHHHHH
    Protocol protocol = Protocol::DSDV; // THIS IS WHERE YOU CHANGE PROTOCOL AAAAAAAAAAAAAAAAAAAAHHHHHHH
    ifstream input;
    input.open(eventsFilename);

    // Set up basic topology properties
    int width;
    int height;
    int radius;
    input >> width;
    input >> height;
    input >> radius;
    input.close();

    // Set up simulation window
    QApplication a(argc, argv);
    QGraphicsView *view = new QGraphicsView();
    QGraphicsScene *scene = new QGraphicsScene();
    view->setScene(scene);
    view->scale(1, -1); // Screen y-axis is inverted (so that y is counted from bottom to top of screen)
    view->setSceneRect(0, 0, width * WINDOW_SCALE, height * WINDOW_SCALE);
    view->show();

    // Set up events and statistics
    StatisticsHandler* statistics = new StatisticsHandler();
    EventHandler* eventHandler = new EventHandler();
    bool eventsDone = false;
    eventHandler->loadEvents(eventsFilename);

    // Counters for the simulation
    int packets = 0; // Only used for printing current packet count
    int time = 0;
    unsigned id = 0;
    int timeDelta;
    int eventDuration = EVENT_DURATION_DEFAULT;

    // Keep track of all hosts
    unordered_map<unsigned, Host*> hosts;

    bool running = true;
    while (running) {
        // Time for when this simulation loop started
        chrono::time_point<std::chrono::system_clock> before = chrono::system_clock::now();

        // If we are done with events and all data packets have either arrived or been dropped
        if (eventsDone && 
            statistics->dataPacketsSent == statistics->dataPacketsArrived + statistics->dataPacketsDropped) {
            running = false;
        }

        // Tick all hosts
        for (auto it : hosts) it.second->tick(time);

        // If 'eventDuration' is equal to or less than 0, move on to the next event
        eventDuration -= TICK_STEP;
        if (eventDuration <= 0) {
            Event* nextEvent = eventHandler->nextEvent(); // Get next event
            if (!nextEvent) { // If there are no more events
                eventsDone = true;
            } 
            else {
                eventDuration = nextEvent->duration;
                switch (nextEvent->eventType) { // Decipher what type of event it is
                    case Event::SEND:
                        // Update statistics if we are sending a packet
                        packets++;
                        statistics->packetsSent++;
                        statistics->dataPacketsSent++;
                        
                        handleSendEvent(nextEvent, &hosts, protocol, time);
                        cout << "Sent packets: " << packets << endl;
                        break;
                    case Event::MOVE:
                        handleMoveEvent(nextEvent, &hosts);
                        break;
                    case Event::JOIN:
                        handleJoinEvent(nextEvent, &hosts, protocol, statistics, radius, time, id);
                        id++;
                        break;
                    case Event::DISCONNECT:
                        handleDisconnectEvent(nextEvent, &hosts);
                        break;
                }
                delete nextEvent;
            }
        }

        // Re-write the simulation window
        scene->clear();
        for (auto it : hosts) {
            it.second->draw(scene);
        }
        view->update();
        a.processEvents();

        // Move forward the correct time
        time += TICK_STEP;
        timeDelta = TICK_INTERVAL - (int) chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - before).count();
        if (timeDelta < 0) timeDelta = 0;

        // Sleep for however long the difference in time it took to run all calculations this simulation tick and 'TIME_INTERVAL'
        this_thread::sleep_for(chrono::milliseconds(timeDelta));
    }

    // Print statistics
    cout << statistics->toString() << endl;

    for (auto it : hosts) { // De-allocate memory
        delete it.second;
    }

    return a.exec();
}

void handleSendEvent(Event* event, unordered_map<unsigned, Host*>* hosts, Protocol protocol, int time) {
    // Get 'h1̈́' and 'h2' from the event
    Host* h1 = (*hosts)[event->senderId];
    Host* h2 = (*hosts)[event->receiverId];

    // Send a packet from 'h1' to 'h2' using the current protocol's specific packet type
    switch (protocol) {
        case DSDV:
            h1->receivePacket(new DSDVPacket(h1, h2, time));
            break;
        case DSR: 
            h1->receivePacket(new DSRPacket(h1, h2, time));
            break;
        case GPSR:
            h1->receivePacket(new GPSRPacket(h1, h2, time));
            break;
    }
}

void handleMoveEvent(Event* event, unordered_map<unsigned, Host*>* hosts) {
    Point* p = new Point(event->x, event->y);
    // Move the host to position 'p'
    (*hosts)[event->hostId]->moveTo(p);
}

void handleJoinEvent(Event* event, unordered_map<unsigned, Host*>* hosts, Protocol protocol, StatisticsHandler* statistics, int radius, int time, unsigned id) {
    Host* newHost;
    int x = event->x;
    int y = event->y;

    // Create a new host of the same type as the current protocol at 'x' and 'y'
    switch (protocol) {
        case DSDV:
            newHost = new DSDVHost(statistics, x, y, radius, time, id, hosts);
            break;
        case DSR:
            newHost = new DSRHost(statistics, x, y, radius, time, id, hosts);
            break;
        case GPSR:
            newHost = new GPSRHost(statistics, x, y, radius, time, id, hosts);
            break;
    }

    // Add the new host to the map of hosts
    hosts->insert(make_pair(newHost->id, newHost));
    newHost->discoverNeighbours();
}

void handleDisconnectEvent(Event* event, unordered_map<unsigned, Host*>* hosts) {
    Host* host = (*hosts)[event->senderId];
    host->die();
    hosts->erase(host->id);
    delete host;
}
