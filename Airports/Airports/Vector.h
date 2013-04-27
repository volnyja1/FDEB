#include "Point.h"

class Vector{
public:
	Vector();
	Vector(Point p1, Point p2);
	Vector(double _x, double _y, double _z);
	void normalize();
	double getLength();
	void multiple(double a);
	void add(Vector v);
	Vector added(Vector v);
	double getX();
	double getY();
	double getZ();
	void setZ(double _z);
private:
	double x, y, z;
};