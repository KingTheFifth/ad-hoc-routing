#include "point.h"
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QPen>
#include <cmath>
#include <sstream>
#include "constants.h"
#include <iomanip>
#include <iostream>

Point::Point(double _x, double _y)
    : x(_x), y(_y) {}

double Point::distanceTo(const Point* that) const
{
    double dx = x - that->x;
    double dy = y - that->y;
    return std::sqrt(dx*dx + dy*dy);
}

void Point::draw(QGraphicsScene *scene, bool special) const
{
    if (special) {
        QGraphicsEllipseItem *item = new QGraphicsEllipseItem(x * WINDOW_SCALE, y * WINDOW_SCALE, 5 * WINDOW_SCALE, 5 * WINDOW_SCALE);
        item->setBrush(QBrush(QColor(0, 255, 255)));
        scene->addItem(item);
    }
    else {
        QGraphicsEllipseItem *item = new QGraphicsEllipseItem(x * WINDOW_SCALE, y * WINDOW_SCALE, 3 * WINDOW_SCALE, 3 * WINDOW_SCALE);
        item->setBrush(QBrush(QColor(255, 0, 0)));
        scene->addItem(item);
    }
}

void Point::drawTo(Point* that, QGraphicsScene *scene) const
{
    QGraphicsLineItem *item = new QGraphicsLineItem(x * WINDOW_SCALE, y * WINDOW_SCALE, that->x * WINDOW_SCALE, that->y * WINDOW_SCALE);
    scene->addItem(item);
}

void Point::drawToAsPerimeter(const Point* that, QGraphicsScene *scene, bool drawxD) const
{
    QGraphicsLineItem *item = new QGraphicsLineItem(x * WINDOW_SCALE, y * WINDOW_SCALE, that->x * WINDOW_SCALE, that->y * WINDOW_SCALE);
    QPen pen;
    // pen.setDashPattern(dashPattern());
    if (drawxD) pen.setColor(Qt::red);
    else pen.setColor(Qt::green);
    pen.setWidth(5);
    item->setPen(pen);
    
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
        return (p->y - y) / (p->x - x);
}

double Point::angleTo(const Point* p) const {
    if (x == p->x && y == p->y)
        return -std::numeric_limits<double>::infinity();  
    double angle = atan2(p->y - y, p->x - x);
    if (angle < 0) return 2*PI + angle;
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

bool operator ==(const Point& p1, const Point& p2) {
    return p1.x == p2.x && p1.y == p2.y;
}

Point* getVerticalCrossing(const Point* vertStart, const Point* vertEnd, const Point* notVertStart, const Point* notVertEnd) {
    // Describe the non-vertical line through notVertStart and notVertEnd on the form y = kx + m
    double k = (notVertEnd->y - notVertStart->y) / (notVertEnd->x - notVertStart->x);
    double m = notVertStart->y - k * notVertStart->x;

    // Calculate the y-coordinate of the point of intersection
    double y = k * vertStart->x + m;

    // If the point of intersection is within the bounds of the vertical line on the y-axis, either 
    // vertStart < y < vertEnd or vertEnd < y < vertStart, which means either vertStart - y is 
    // negative or vertEnd - y is negative. The same reasoning can be applied to check if the point 
    // is within the bounds of the non-vertical line on the x-axis
    bool withinVertY = (vertStart->y - y) * (vertEnd->y - y) < 0;
    bool withinNotVertX = (notVertStart->x - vertStart->x) * (notVertEnd->x - vertStart->x) < 0;
    
    if (withinVertY && withinNotVertX) return new Point(vertStart->x, y);
    return nullptr;
}

Point* getCrossing(const Point* const A1, const Point* const A2, const Point* const B1, const Point* const B2) {
    // If the end point of one 'line' is the start point of the other
    // count it as if the lines do not intersect
    if (A2 == B1 || B2 == A1) {
        return nullptr;
    }

    // If both 'lines' are vertical (and thus parallell)
    if (A1->x == A2->x && B1->x == B2->x) {
        return nullptr;
    }

    // If 'line A' is vertical but 'line B' is not
    if (A1->x == A2->x) {
        return getVerticalCrossing(A1, A2, B1, B2);
    }

    // If 'line B' is vertical but 'line A' is not
    if (B1->x == B2->x) {
        return getVerticalCrossing(B1, B2, A1, A2);
    }

    // Describe both lines on the form y = kx + m
    double kA = (A2->y - A1->y) / (A2->x - A1->x);
    double mA = A1->y - kA * A1->x;

    double kB = (B2->y - B1->y) / (B2->x - B1->x);
    double mB = B1->y - kB * B1->x;

    // If the slope of both lines are the same, they are parallel and thus do not intersect
    if (kA == kB) {
        return nullptr;
    }

    // Get the x-coordinate of the point of intersection by setting both lines equal to each
    // other and solving for x
    double x = (mA - mB) / (kB - kA);

    // If the point of intersection is within the bounds of 'line A' on the x-axis, either 
    // A1 < x < A2 or A2 < x < A1, which means either A1 - x is negative or A2 - x is negative.
    // The same reasoning can be applied to check if the point is within the bounds of 'line B'
    bool withinA = (A1->x - x) * (A2->x - x) < 0;
    bool withinB = (B1->x - x) * (B2->x - x) < 0;
    
    if (withinA && withinB) return new Point(x, kA * x + mA);
    return nullptr;
}
