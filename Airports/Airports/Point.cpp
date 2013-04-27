#include "Point.h"

Point::Point(){

}

Point::~Point(){

}

Point::Point(double _x, double _y, double _z){
	x = _x;
	y = _y;
	z = _z;
}

void Point::setPoint(Point p){
	x = p.getX();
	y = p.getY();
	z = p.getZ();
}

void Point::draw(){

}

double Point::getX(){
	return x;
}

double Point::getY(){
	return y;
}

double Point::getZ(){
	return z;
}

void Point::setZ(double _z){
	z = _z;
}