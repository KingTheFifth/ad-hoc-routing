#include "point.h"
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <cmath>
#include <sstream>
#include "constants.h"
#include <iomanip>
#include <iostream>

Point::Point(int _x, int _y)
    : x(_x), y(_y) {}

double Point::distanceTo(const Point* that) const
{
    double dx = x - that->x;
    double dy = y - that->y;
    return std::sqrt(dx*dx + dy*dy);
}

void Point::draw(QGraphicsScene *scene) const
{
    QGraphicsEllipseItem *item = new QGraphicsEllipseItem(x, y, 3 * WINDOW_SCALE, 3 * WINDOW_SCALE);
    item->setBrush(QBrush(QColor(255, 0, 0)));
    scene->addItem(item);
}

void Point::drawTo(Point* that, QGraphicsScene *scene) const
{
    QGraphicsLineItem *item = new QGraphicsLineItem(x, y, that->x, that->y);
    scene->addItem(item);
}

double Point::slopeTo(const Point* p) const {
    if (x == p->x && y == p->y)
        return  -std::numeric_limits<double>::infinity();
    else if (y == p->y) // horizontal line segment
        return 0.0;
    else if (x == p->x) // vertical line segment
        return  std::numeric_limits<double>::infinity();
    else
        return (static_cast<double>(p->y) - static_cast<double>(y)) /
                (static_cast<double>(p->x) - static_cast<double>(x));
}

double Point::angleTo(const Point* p) const {
    if (x == p->x && y == p->y)
        return -std::numeric_limits<double>::infinity();  
    double angle = atan2(p->x - x, p->y - y);
    if (angle < 0) return PI - angle;
    return angle;
}

string Point::toString() const {
    stringstream string;
    string << "(" << std::fixed << std::setprecision(1) << std::showpoint
        << x << ", " << y << ")";
    return string.str();
}

ostream& operator <<(ostream& out, const Point& p) {
    out << p.toString();
    return out;
}