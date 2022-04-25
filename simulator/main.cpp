#include <QApplication>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "host.h"

using namespace std;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QGraphicsView *view = new QGraphicsView();
    QGraphicsScene *scene = new QGraphicsScene();

    // Set up graphics
    int width = 500;
    int height = 500;
    view->setScene(scene);
    view->scale(1, -1); //screen y-axis is inverted
    view->setSceneRect(0, 0, width, height);
    view->show();

    string topologyFileName = "top.txt";
    ifstream input;
    input.open(topologyFileName);

    vector<Host*> hosts;

    // get dimensions
    // int width;
    // int height;
    input >> width;
    input >> height;

    int radius = 50; // TODO: Do something about the radius

    int x;
    int y;
    while(input >> x >> y) {
        hosts.push_back(new Host(x, y, radius));
    }
    input.close();

    // let all hosts find their neighbours
    for (auto& host : hosts) {
        host->discoverNeighbours(&hosts);
    }

    // draw the thing!
    for (auto& host : hosts) {
        host->draw(scene);
    }

    // scene->clear();
    view->update();
    a.processEvents(); // (?)

    return a.exec();
}
