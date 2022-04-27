#include "point.h"
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <cmath>

Point::Point(int _x, int _y)
    : x(_x), y(_y) {}


double Point::distanceTo(Point that) const
{
    double dx = x - that.x;
    double dy = y - that.y;
    return std::sqrt(dx*dx + dy*dy);
}

void Point::draw(QGraphicsScene *scene) const
{
    QGraphicsEllipseItem *item = new QGraphicsEllipseItem(x, y, 1, 1);
    item->setBrush(QBrush(QColor(255, 0, 0)));
    scene->addItem(item);
}

void Point::drawTo(Point that, QGraphicsScene *scene) const
{
    QGraphicsLineItem *item = new QGraphicsLineItem(x, y, that.x, that.y);
    scene->addItem(item);
}

double Point::slopeTo(const Point& p) const {
    if (x == p.x && y == p.y)
        return  -std::numeric_limits<double>::infinity();
    else if (y == p.y) // horizontal line segment
        return 0.0;
    else if (x == p.x) // vertical line segment
        return  std::numeric_limits<double>::infinity();
    else
        return (static_cast<double>(p.y) - static_cast<double>(y)) /
                (static_cast<double>(p.x) - static_cast<double>(x));
}