#include "Vector.h"
#include <math.h>

Vector::Vector(){
	x = 0.0;
	y = 0.0;
	z = 0.0;
}

Vector::Vector(Point p1, Point p2){
	x = p2.getX()-p1.getX();
	y = p2.getY()-p1.getY();
	z = p2.getZ()-p1.getZ();
}

Vector::Vector(double _x, double _y, double _z){
	x = _x;
	y = _y;
	z = _z;
}

void Vector::normalize(){
	double l = getLength();
	x /= l;
	y /= l;
	z /= l;
}

void Vector::multiple(double a){
	x *= a;
	y *= a;
	z *= a;
}

void Vector::add(Vector v){
	x += v.getX();
	y += v.getY();
	z += v.getZ();
}

Vector Vector::added(Vector v){
	return Vector(x+v.getX(), y+v.getY(), z+v.getZ());
}

double Vector::getLength(){
	return sqrt((x*x)+(y*y)+(z*z));
}

double Vector::getX(){
	return x;
}

double Vector::getY(){
	return y;
}

double Vector::getZ(){
	return z;
}

void Vector::setZ(double _z){
	z = _z;
}