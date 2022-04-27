#include <QApplication>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include "host.h"

using namespace std;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QGraphicsView *view = new QGraphicsView();
    QGraphicsScene *scene = new QGraphicsScene();

    string topologyFileName = "shrimp.txt";
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

    view->setScene(scene);
    view->scale(1, -1); //screen y-axis is inverted
    view->setSceneRect(0, 0, width, height);
    view->show();

    // Set up initial topology
    int x;
    int y;
    int time = 0;
    while(input >> x >> y) {
        hosts.push_back(new Host(x, y, radius, time));
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


    //unsigned five = 5;
    //unsigned six = 6;
    //cout << "Cool experiment: " << five - six << endl;
    int packets = 0;
    while (true) { // Simulation is running (TODO: Do something different here)
        //chrono::milliseconds before(chrono::system_clock::now());
        chrono::time_point<std::chrono::system_clock> before = chrono::system_clock::now();

        //chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
        for (auto& host : hosts) {
            host->tick(time);
        }

        if (time % 100 == 0) {
            int rndindex = rand() % hosts.size();
            Host* h1 = hosts[rndindex];

            rndindex = rand() % hosts.size();
            Host* h2 = hosts[rndindex];

            rndindex = rand() % h1->neighbours.size();
            Link* l = h1->neighbours[rndindex];
            h1->forwardPacket(new Packet(h1, h2), l);
            packets++;
            cout << "Packets: " << packets << endl;
        }

        scene->clear();
        for (auto& host : hosts) {
            host->draw(scene);
        }
        view->update();
        a.processEvents();
        time++;
        int timeDelta = 5 - (int) chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - before).count();
        if (timeDelta < 0) timeDelta = 0;

        this_thread::sleep_for(chrono::milliseconds(timeDelta));
    }

    return a.exec();
}
