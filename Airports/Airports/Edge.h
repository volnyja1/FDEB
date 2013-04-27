#include "Airport.h"
#include <vector>

class Edge{
public:
	Edge();
	Edge(int _id, int _source, int _target);
	Edge(int _id, Airport* _source, Airport* _target);
	Edge(Point i1, Point i2);
	~Edge();
	void setEdge(int _id, int _source, int _target);
	int getID();
	int getSourceID();
	int getTargetID();
	Airport* getSource();
	Airport* getTarget();
	void recalculatePoints();
	void decreasePoints();
	void draw();
	double getLength();
	std::vector<Point> getPoints();
	void setPoint(Point p, int i);
	Point getMidPoint();
	void restartEdge();
private:
	int source, target, id;
	double length;
	Airport* start;
	Airport* end;
	std::vector<Point> points;
};