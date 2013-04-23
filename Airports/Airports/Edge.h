#include "Airport.h"

class Edge{
public:
	Edge(int _id, int _source, int _target);
	~Edge();
	void setEdge(int _id, int _source, int _target);
	int getID();
	int getSourceID();
	int getTargetID();
	Airport* getSource();
	Airport* getTarget();
private:
	int source, target, id;
	Airport* start;
	Airport* end;
};