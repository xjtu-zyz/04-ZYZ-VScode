#ifndef __HEIGHTCALAULATOR_H__
#define __HEIGHTCALAULATOR_H__

#define M_PI        3.14159265358979323846
#include <iostream>
#include <vector>
#include <iomanip>
using namespace std;

class Point
{
public:
	Point(double fx, double fy, double fz)
		:x(fx), y(fy), z(fz)
	{
	}
	Point(){x=0;y=0;z=0;};

	// Subtract
	Point operator - (const Point& v) const
	{
		return Point(x - v.x, y - v.y, z-v.z) ;
	}

	// Dot product
	double Dot(const Point& v) const
	{
		return x * v.x + y * v.y ;
	}

public:
	double x, y, z ;
};
struct Traingle
{
	Point *A;
	Point *B;
	Point *C;
	double maxx;
	double minx;
	double maxy;
	double miny;
public:
	Traingle()
	{
		A = NULL;
		B = NULL;
		C = NULL;
	}
};



class HeightCalculator
{
public:
	vector<Traingle*> m_vecTraingles;
	Point *m_point;
	vector<Point*> m_vecdots;
	Traingle * m_Traingle;
public:
	HeightCalculator(void);
	~HeightCalculator(void);
public:
	void LoadTrigle();
	void LoadPoints();
	void CalculatePointsHeight();

public:
	void SetTrgleMinMaxXY(Traingle * trgle);
	double* MaxMin(double a, double b, double c);
	bool LocatePoint(Point *p);
	bool PointinTriangle(Point* A, Point* B, Point* C, Point* P);
};

void split(std::string& s, std::string& delim,std::vector< std::string >& ret) ;
#endif