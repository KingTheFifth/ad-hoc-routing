#ifndef POINT_H
#define POINT_H

#include <QGraphicsScene>

struct Point {
    int x;
    int y;

    Point(int _x, int _y);

    void draw(QGraphicsScene *scene) const;

    void drawTo(Point* that, QGraphicsScene *scene) const;

    double distanceTo(Point* that) const;

    double slopeTo(const Point* p) const;

    double angleTo(const Point* p) const;
};

#endif