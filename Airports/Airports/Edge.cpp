#include "Edge.h"

Edge::Edge(int _id, int _source, int _target){
	id = _id;
	source = _source;
	target = _target;
}

Edge::~Edge(){

}

void Edge::setEdge(int _id, int _source, int _target){
	id = _id;
	source = _source;
	target = _target;
}

int Edge::getID(){
	return id;
}

int Edge::getSourceID(){
	return source;
}

int Edge::getTargetID(){
	return target;
}

Airport* Edge::getSource(){
	return start;
}

Airport* Edge::getTarget(){
	return end;
}