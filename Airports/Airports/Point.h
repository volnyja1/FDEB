class Point{
public:
	Point();
	~Point();
	Point(double _x, double _y, double _z);
	void setPoint(Point p);
	void draw();
	double getX();
	double getY();
	double getZ();
	void setZ(double _z);
private:
	double x, y, z;
};