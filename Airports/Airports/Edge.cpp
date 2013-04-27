#include "Edge.h"
#include <math.h>

Edge::Edge(){
	length = 0.0;
}

Edge::Edge(int _id, int _source, int _target){
	id = _id;
	source = _source;
	target = _target;
	length = 0.0;
}

Edge::Edge(int _id, Airport* _source, Airport* _target){
	id = _id;
	source = _source->getID();
	target = _target->getID();
	start = _source;
	end = _target;
	points.push_back(Point(start->getX(), start->getY(), start->getZ()));
	points.push_back(Point(end->getX(), end->getY(), end->getZ()));
	length = sqrt((end->getX()-start->getX())*(end->getX()-start->getX())+
				  (end->getY()-start->getY())*(end->getY()-start->getY()));
}

Edge::Edge(Point i1, Point i2){
	start = new Airport(0,i1.getX(),i1.getY(),i1.getZ(),"");
	end = new Airport(0,i2.getX(),i2.getY(),i2.getZ(),"");
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

void Edge::recalculatePoints(){
	std::vector<Point>temp;
	if(points.size() == 2){
		temp.push_back(points[0]);
		temp.push_back(Point(points[0].getX()+(points[1].getX()-points[0].getX())/2,
							 points[0].getY()+(points[1].getY()-points[0].getY())/2,
							 length/15.0));
		temp.push_back(points[1]);
	} else{
		for(unsigned int i = 0; i < points.size(); i++){
			if(i==points.size()-1){
				temp.push_back(Point(points[i].getX(),
									 points[i].getY(),
									 0.0));
			} else{
				temp.push_back(Point(points[i].getX(),
									 points[i].getY(),
									 length/15.0*sin(3.14*((2.0*(double)i)/(((double)points.size())*2.0-1.0)))));
			}
			if(i != points.size()-1)
				temp.push_back(Point(points[i].getX()+(points[i+1].getX()-points[i].getX())/2,
									 points[i].getY()+(points[i+1].getY()-points[i].getY())/2,
									 length/15.0*sin(3.14*((2.0*((double)i+1.0)-1.0)/((double)points.size()*2.0-1.0)))));
		}
	}
	points.clear();
	for(unsigned int i = 0; i < temp.size(); i++){
		points.push_back(temp[i]);
	}
}

void Edge::decreasePoints(){
	std::vector<Point>temp;
	if(points.size() != 2){
		for(unsigned int i = 0; i < points.size(); i++){
			if(i%2 == 0){
				temp.push_back(Point(points[i].getX(),points[i].getY(),
					length/15.0*sin(3.14*(((double)i/2.0)/(((double)points.size()+1.0)/2.0-1.0)))));
			}
		}
		points.clear();
		for(unsigned int i = 0; i < temp.size(); i++){
			points.push_back(temp[i]);
		}
	}
}

void Edge::restartEdge(){
	std::vector<Point>temp;
	temp.push_back(points[0]);
	temp.push_back(points[points.size()-1]);
	points.clear();
	for(unsigned int i = 0; i < temp.size(); i++){
		points.push_back(temp[i]);
	}
}

double Edge::getLength(){
	return length;
}

std::vector<Point> Edge::getPoints(){
	return points;
}

void Edge::setPoint(Point p, int i){
	points[i].setPoint(p);
}

Point Edge::getMidPoint(){
	return Point(end->getX()-start->getX(), end->getY()-start->getY(), end->getZ()-start->getZ());
}