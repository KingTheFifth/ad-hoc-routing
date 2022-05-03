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

using namespace std;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QGraphicsView *view = new QGraphicsView();
    QGraphicsScene *scene = new QGraphicsScene();

    string topologyFileName = "portal.txt";
    ifstream input;
    input.open(topologyFileName);
    
    vector<Host*> hosts;

    // Set up graphics
    int width;
    int height;
    int radius;

    input >> width;
    input >> height;
    input >> radius;
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

    enum Protocol {DSDV, DSR, GPSR};
    Protocol protocol = Protocol::DSR;

    while(input >> x >> y) {
        Host* host;
        switch (protocol) {
            case DSDV:
                host = new DSDVHost(x, y, radius, time, id);
                break;
            case DSR: 
                host = new DSRHost(x, y, radius, time, id);
                break;
            case GPSR:
                host = new GPSRHost(x, y, radius, time, id);
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
    while (true) { // Simulation is running (TODO: Do something different here)
        chrono::time_point<std::chrono::system_clock> before = chrono::system_clock::now();
        //cout << "Crossing: " << *getCrossing(new Point(0,0), new Point(5,5), new Point(2, 0), new Point(2,5)) << endl;

        for (auto& host : hosts) {
            host->tick(time);
        }

        if (ONLY_ONE_PACKET == 0 && time % 100 == 0) {
            int rndindex = rand() % hosts.size();
            Host* h1 = hosts[rndindex];

            rndindex = rand() % hosts.size();
            Host* h2 = hosts[rndindex];
            while (h2 == h1) {
                rndindex = rand() % hosts.size();
                h2 = hosts[rndindex];
            }
            
            h1->receivePacket(new DSRPacket(h1, h2)); // TODO: temporarily uses GPSR for now
            packets++;
            
            if (packets % 10 == 0) { cout << "Packets: " << packets << endl; }
            
            // cout << "Packets: " << packets << endl;
        }
        if (ONLY_ONE_PACKET == 1 && time == TICK_STEP) {
            sender->receivePacket(new DSRPacket(sender, receiver));
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

    return a.exec();
}
