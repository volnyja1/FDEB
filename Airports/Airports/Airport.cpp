#include "Airport.h"

Airport::Airport(int _id, double _x, double _y, double _z, std::string _name){
	id = _id;
	x = _x;
	y = _y;
	z = _z;
	name = _name;
	degree = 0;
	selected = false;
}

Airport::~Airport(){

}

void Airport::setAirport(int _id, double _x, double _y, double _z, std::string _name){
	id = _id;
	x = _x;
	y = _y;
	z = _z;
	name = _name;
	degree = 0;
	selected = false;
}

int Airport::getID(){
	return id;
}

int Airport::getDegree(){
	return degree;
}

void Airport::setDegree(int _degree){
	degree = _degree;
}

void Airport::increaseDegree(){
	degree++;
}

double Airport::getX(){
	return x;
}

double Airport::getY(){
	return y;
}

double Airport::getZ(){
	return z;
}

std::string Airport::getName(){
	return name;
}

void Airport::setSelected(bool _selected){
	selected = _selected;
}

bool Airport::getSelected(){
	return selected;
}