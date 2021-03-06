#include <string>
#include "Vector.h"

class Airport{
public:
	Airport(int _id, double _x, double _y, double _z, std::string _name);
	~Airport();
	void setAirport(int _id, double _x, double _y, double _z, std::string _name);
	int getID();
	int getDegree();
	void setDegree(int _degree);
	void increaseDegree();
	double getX();
	double getY();
	double getZ();
	std::string getName();
	void setSelected(bool _selected);
	bool getSelected();
private:
	std::string name;
	double x, y, z;
	int degree, id;
	bool selected;
};