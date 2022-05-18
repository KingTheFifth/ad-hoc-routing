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

void handleSendEvent(Event* event, unordered_map<unsigned, Host*>* hosts, Protocol protocol, int time);
void handleMoveEvent(Event* event, unordered_map<unsigned, Host*>* hosts);
void handleJoinEvent(Event* event, unordered_map<unsigned, Host*>* hosts, Protocol protocol, StatisticsHandler* statistics, int radius, int time, unsigned id);
void handleDisconnectEvent(Event* event, unordered_map<unsigned, Host*>* hosts);

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QGraphicsView *view = new QGraphicsView();
    QGraphicsScene *scene = new QGraphicsScene();

    string eventsFilename = "ev_small_dense_move.txt"; // THIS IS WHERE YOU CHANGE TOPOLOGY/EVENT SETS AAAAAAAAAAAAAAAAAHHHHHHHH
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

    view->setScene(scene);
    view->scale(1, -1); // Screen y-axis is inverted (so that y is counted from bottom to top of screen)
    view->setSceneRect(0, 0, width * WINDOW_SCALE, height * WINDOW_SCALE);
    view->show();
    scene->setItemIndexMethod(QGraphicsScene::ItemIndexMethod::NoIndex);

    Protocol protocol = Protocol::DSR; // THIS IS WHERE YOU CHANGE PROTOCOL AAAAAAAAAAAAAAAAAAAAHHHHHHH
    StatisticsHandler* statistics = new StatisticsHandler();
    EventHandler* eventHandler = new EventHandler();
    bool eventsDone = false;

    eventHandler->loadEvents(eventsFilename);

    int packets = 0; // Only used for printing current packet count
    int time = 0;
    unsigned id = 0;
    int timeDelta;
    int eventDuration = EVENT_DURATION_DEFAULT;
    unordered_map<unsigned, Host*> hosts;

    Host* sender;
    Host* receiver;
    if (ONLY_ONE_PACKET) { // DEBUG
        sender = hosts[2];
        receiver = hosts[8];
    }

    bool running = true;
    while (running) {
        chrono::time_point<std::chrono::system_clock> before = chrono::system_clock::now();

        if (eventsDone && statistics->dataPacketsSent == statistics->dataPacketsArrived + statistics->dataPacketsDropped) {
            running = false;
        }

        for (auto it : hosts) it.second->tick(time);

        eventDuration -= TICK_STEP;


        if (ONLY_ONE_PACKET == 0 && eventDuration <= 0) {
            Event* nextEvent = eventHandler->nextEvent();
            if (!nextEvent) {
                eventsDone = true;
            } else {
                eventDuration = nextEvent->duration;
                switch (nextEvent->eventType) {
                    case Event::SEND:
                        packets++;
                        statistics->packetsSent++;
                        statistics->dataPacketsSent++;
                        handleSendEvent(nextEvent, &hosts, protocol, time);
                        //if (packets % 10 == 0) { 
                            cout << "Packets: " << packets << endl; // TODO: Remove this
                        //}
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
        else if (ONLY_ONE_PACKET == 1 && time == TICK_STEP) { // DEBUG
            switch (protocol) {
                case DSDV:
                    sender->receivePacket(new DSDVPacket(sender, receiver, time));
                    break;
                case DSR: 
                    sender->receivePacket(new DSRPacket(sender, receiver, time));
                    break;
                case GPSR:
                    sender->receivePacket(new GPSRPacket(sender, receiver, time));
                    break;
            }
        }

        scene->clear();
        for (auto it : hosts) {
            it.second->draw(scene);
        }
        
        if (ONLY_ONE_PACKET) {
            // sender->getPos()->draw(scene, true);
            // receiver->getPos()->draw(scene, true);
        }

        view->update();
        a.processEvents();
        time += TICK_STEP;
        timeDelta = TICK_INTERVAL - (int) chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - before).count();
        if (timeDelta < 0) timeDelta = 0;

        this_thread::sleep_for(chrono::milliseconds(timeDelta));
    }

    cout << statistics->toString() << endl;

    for (auto it : hosts) {
        delete it.second;
    }

    return a.exec();
}

void handleSendEvent(Event* event, unordered_map<unsigned, Host*>* hosts, Protocol protocol, int time) {
    // TODO: consider the size of data, send multiple packets (if we change throughput to bytes instead of packets)

    Host* h1 = (*hosts)[event->senderId];
    Host* h2 = (*hosts)[event->receiverId];
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
    (*hosts)[event->hostId]->moveTo(p);
}

void handleJoinEvent(Event* event, unordered_map<unsigned, Host*>* hosts, Protocol protocol, StatisticsHandler* statistics, int radius, int time, unsigned id) {
    Host* newHost;
    int x = event->x;
    int y = event->y;
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
    hosts->insert(make_pair(newHost->id, newHost));
    newHost->discoverNeighbours();
}

void handleDisconnectEvent(Event* event, unordered_map<unsigned, Host*>* hosts) {
    Host* host = (*hosts)[event->senderId];
    host->die();
    hosts->erase(host->id);
    delete host;
}
