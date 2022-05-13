#ifndef POINT_H
#define POINT_H

#include <QGraphicsScene>
#include <string>
#include <iostream>

using namespace std;

struct Point {
	double x;
	double y;

	Point(double _x, double _y);

	/**
	 * Draw this point
	 */
	void draw(QGraphicsScene *scene, bool special = false) const; // TODO: remove bool sometime later

	/**
	 * Draw a line from this point to 'that'
	 */
	void drawTo(Point* that, QGraphicsScene *scene) const;

	/**
	 * Draw a line from this point to 'that'
	 */
	void drawToAsPerimeter(const Point* that, QGraphicsScene *scene, bool drawxD) const;

	/**
	 * Get the distance from this point to 'that'
	 */
	double distanceTo(const Point* that) const;

	/**
	 * 
	 */
	double slopeTo(const Point* p) const;

	/**
	 * Calculate the angle from this point to the point 'p' relative to this points positive x-axis
	 */
	double angleTo(const Point* p) const;

	/**
	 * To string method for the Point struct
	 */
	string toString() const;
};

ostream& operator<<(ostream& out, const Point& p);

/**
 * Returns the point of intersection between two lines A and B, where A starts at point A1
 * and ends at point A2, and B starts at point B1 and ends at point B2. Returns nullptr
 * if the lines do not intersect. Lines with equal start or end points do not count as intersecting.
 * NB: Allocates the point of intersection.
 */
Point* getCrossing(const Point* const A1, const Point* const A2, const Point* const B1, const Point* const B2);

/**
 * Returns the point of intersection between two lines A and B, where A is vertical and starts at point vertStart
 * and ends at point vertEnd, and B is not vertical and starts at point notVertStart and ends at point notVertEnd.
 * Returns nullptr if the lines do not intersect. NB: Allocates the point of intersection.
 */
Point* getVerticalCrossing(const Point* vertStart, const Point* vertEnd, const Point* notVertStart, const Point* notVertEnd);

#endif