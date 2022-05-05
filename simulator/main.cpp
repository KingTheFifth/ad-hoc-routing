#include <QApplication>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include "host/host.h"
#include "host/GPSRHost.h"
#include "host/DSRHost.h"
#include "host/DSDVHost.h"
#include "point.h"
#include "constants.h"
#include "StatisticsHandler.h"
#include "events/EventHandler.h"
#include "events/Event.h"

using namespace std;

enum Protocol {DSDV, DSR, GPSR};

void handleSendEvent(Event* event, vector<Host*>* hosts, Protocol protocol, int time);
void handleMoveEvent(Event* event, vector<Host*>* hosts);
void handleJoinEvent(Event* event, vector<Host*>* hosts, Protocol protocol, StatisticsHandler* statistics, int radius, int time, unsigned id);
void handleDisconnectEvent(Event* event, vector<Host*>* hosts);

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QGraphicsView *view = new QGraphicsView();
    QGraphicsScene *scene = new QGraphicsScene();

    string topologyFilename = "portal.txt";
    string eventsFilename = "events.txt";
    ifstream input;
    input.open(topologyFilename);
    
    vector<Host*> hosts;

    // Set up graphics
    int width;
    int height;
    int radius;
    int simulationTime;

    input >> width;
    input >> height;
    input >> radius;
    input >> simulationTime;
    width *= WINDOW_SCALE;
    height *= WINDOW_SCALE;

    view->setScene(scene);
    view->scale(1, -1); //screen y-axis is inverted
    view->setSceneRect(0, 0, width, height);
    view->show();

    // Set up initial topology
    int x;
    int y;
    int time = 0;
    unsigned id = 0;

    int maxNumberOfPackets = 60; // temporary :)
    Protocol protocol = Protocol::DSR;
    StatisticsHandler* statistics = new StatisticsHandler();
    EventHandler* eventHandler = new EventHandler();
    int quitDelay = 10000;
    bool eventsDone = false;

    eventHandler->loadEvents(eventsFilename);

    while(input >> x >> y) {
        Host* host;
        switch (protocol) {
            case DSDV:
                host = new DSDVHost(statistics, x, y, radius, time, id);
                break;
            case DSR: 
                host = new DSRHost(statistics, x, y, radius, time, id);
                break;
            case GPSR:
                host = new GPSRHost(statistics, x, y, radius, time, id);
                break;
        }
        hosts.push_back(host);
        id++;
    }
    input.close();
    
    // let all hosts find their neighbours
    for (auto& host : hosts) {
        host->discoverNeighbours(&hosts);
    }

    // draw the network
    for (auto& host : hosts) {
        host->draw(scene);
    }

    int packets = 0;
    int timeDelta;

    Host* sender;
    Host* receiver;
    if (ONLY_ONE_PACKET) {
        sender = hosts[8];
        receiver = hosts[39];
    }
    while (time < simulationTime) { // Simulation is running (TODO: Do something different here. Simulation time in input file?)
        chrono::time_point<std::chrono::system_clock> before = chrono::system_clock::now();

        for (auto& host : hosts) host->tick(time);

        if (eventsDone) quitDelay -= TICK_STEP;
        if (quitDelay <= 0) break;

        if (ONLY_ONE_PACKET == 0 && time % 80 == 0) {
            Event* nextEvent = eventHandler->nextEvent();
            if (!nextEvent)
                eventsDone = true;
            else {
                cout << "New event of type " << nextEvent->eventType << endl;
                switch (nextEvent->eventType) {
                    case Event::SEND:
                        packets++;
                        handleSendEvent(nextEvent, &hosts, protocol, time);
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
            
                // packets++;
                if (packets % 10 == 0) { cout << "Packets: " << packets << endl; }
            }
        }
        else if (ONLY_ONE_PACKET == 1 && time == TICK_STEP) { // DEBUG
            switch (protocol) {
                case DSDV:
                    // sender->receivePacket(new DSDVPacket(sender, receiver, time));
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
        for (auto& host : hosts) {
            host->draw(scene);
        }
        
        if (ONLY_ONE_PACKET) {
            sender->getPos()->draw(scene, true);
            receiver->getPos()->draw(scene, true);
        }

        view->update();
        a.processEvents();
        time += TICK_STEP;
        timeDelta = TICK_SPEED - (int) chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - before).count();
        if (timeDelta < 0) timeDelta = 0;

        this_thread::sleep_for(chrono::milliseconds(timeDelta));
    }

    cout << statistics->toString() << endl;

    return a.exec();
}

void handleSendEvent(Event* event, vector<Host*>* hosts, Protocol protocol, int time) {
    // TODO: consider the size of data, send multiple packets

    Host* h1 = (*hosts)[event->senderId];
    Host* h2 = (*hosts)[event->receiverId];
    switch (protocol) {
        case DSDV:
            // h1->receivePacket(new DSDVPacket(h1, h2, time));
            break;
        case DSR: 
            h1->receivePacket(new DSRPacket(h1, h2, time));
            break;
        case GPSR:
            h1->receivePacket(new GPSRPacket(h1, h2, time));
            break;
    }
}

void handleMoveEvent(Event* event, vector<Host*>* hosts) {
    cout << "Out of the way! We are moving!" << endl;
    Point* p = new Point(event->x, event->y);
    (*hosts)[event->hostId]->moveTo(p);
}

void handleJoinEvent(Event* event, vector<Host*>* hosts, Protocol protocol, StatisticsHandler* statistics, int radius, int time, unsigned id) {
    Host* newHost;
    int x = event->x;
    int y = event->y;
    switch (protocol) {
        case DSDV:
            // newHost = new DSDVHost(statistics, x, y, radius, time, id);
            break;
        case DSR:
            newHost = new DSRHost(statistics, x, y, radius, time, id);
            break;
        case GPSR:
            newHost = new GPSRHost(statistics, x, y, radius, time, id);
            break;
    }
    hosts->push_back(newHost);
    newHost->discoverNeighbours(hosts);
}

void handleDisconnectEvent(Event* event, vector<Host*>* hosts) {
    return;
}