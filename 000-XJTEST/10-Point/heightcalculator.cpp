#include "heightcalculator.h"
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

HeightCalculator::HeightCalculator()
{
	m_point = new Point();
	m_Traingle = new Traingle();
}

HeightCalculator::~HeightCalculator()
{
	if (m_vecTraingles.size() !=0)
	{
		vector<Traingle*>::iterator dit = m_vecTraingles.begin();
		Traingle* trgle = *dit;
		delete trgle->A;
		delete trgle->B;
		delete trgle->C;
		delete trgle;
		dit = m_vecTraingles.erase(dit);
	}
	delete m_Traingle;
	if (m_vecdots.size() !=0 )
	{
		vector<Point *>::iterator pit = m_vecdots.begin();
		Point * point = *pit;
		delete point;
		pit = m_vecdots.erase(pit);
	}
}

void HeightCalculator::LoadTrigle()
{
	ifstream inFile("triangle.txt", ios::in);  
	string line;  
	int i = 0;
	while (getline(inFile, line))  
	{  
		i++;
		vector<Point *> threepoints;
		for (int times = 0; times < 3; times++)
		{
			if (getline(inFile,line))
			{
				if (line.size()>1)
				{
					vector< string > ret;
					string delim = ",";
					split(line,delim,ret);
					if (ret.size() != 3)
					{
						cout << "bad triangle data at line: " << i << endl;
					}else
					{
						Point *A = new Point;
						string strx = ret[0];
						string stry = ret[1];
						string strz = ret[2];
						A->x = atof(strx.c_str());
						A->y = atof(stry.c_str());
						A->z = atof(strz.c_str());
						threepoints.push_back(A);
					}
				}else
				{
					cout << "bad triangle data around line: " << i << endl;
				}
			}
			i++;
		}
		
		if (threepoints.size() == 3)
		{
			Traingle *trgl = new Traingle;
			trgl->A = threepoints[0];
			trgl->B = threepoints[1];
			trgl->C = threepoints[2];
			//make bounding box of this traingle
			SetTrgleMinMaxXY(trgl);

			m_vecTraingles.push_back(trgl);
		
		}else
		{
			cout << "bad triangle data around line: " << i << endl;
		}
	}  
	inFile.close();
}

void HeightCalculator::LoadPoints()
{
	ifstream inFile("point.txt", ios::in);  
	string line;
	int i=0;
	while (getline(inFile, line))  
	{  
		Point *tmpd = new Point;
		vector< string > ret;
		string delim = ",";
		split(line,delim,ret);
		if (ret.size() != 2)
		{
			cout << "bad point data at line: " << i << endl;
		}else
		{
			string strx = ret[0];
			string stry = ret[1];
			tmpd->x = atof(strx.c_str());
			tmpd->y = atof(stry.c_str());

			m_vecdots.push_back(tmpd);
		}
		i++;
	}  

	inFile.close();
}

void HeightCalculator::SetTrgleMinMaxXY(Traingle * trgle)
{
	double *x = NULL;
	x = MaxMin(trgle->A->x, trgle->B->x, trgle->C->x);
	trgle->maxx = x[0];
	trgle->minx = x[1];
	x = MaxMin(trgle->A->y, trgle->B->y, trgle->C->y);
	trgle->maxy = x[0];
	trgle->miny = x[1];
	
	delete[] x;
}

double* HeightCalculator::MaxMin(double a, double b, double c)
{
	double max, min;
	if(a > b)  
	{
		max = a;
		min = b;
	}else 
	{
		max = b;
		min = a; 
	}
		   
	if(c > max)
	{
		max = c;
	}	  
	if (c < min) 
	{
		min = c; 
	}	 
	double* maxmin = new double[2];
	maxmin[0] = max;
	maxmin[1] = min;
	return maxmin;
}

void HeightCalculator::CalculatePointsHeight()
{
	vector<Point *>::iterator pit = m_vecdots.begin();
	ofstream heightFile;
	heightFile.open("point_height.csv", ios::out);

	while(pit != m_vecdots.end())
	{
		Point *p = *pit;
		if (LocatePoint(p))
		{
			heightFile << fixed <<"x=" << p->x <<"," << "y=" << p->y << ","<<"height=" <<p->z << endl;

		}else
		{
			heightFile << fixed << "point: x = " << p->x <<", y = " << p->y << " is not in this area." << endl;
		}
		
		pit++;
	}
}

bool HeightCalculator::LocatePoint(Point *p)
{
	vector<Traingle *>::iterator tit = m_vecTraingles.begin();
	while(tit != m_vecTraingles.end())
	{
		Traingle * trgle = *tit;

		//using bounding box first
		if (p->x >= trgle->minx && p->x <= trgle->maxx
			&& p->y >= trgle->miny && p->y <= trgle->maxy)
		{
			//using weight method to find out whether the point is in this traingle
			//if it is, caculate its height and print the height
			if (PointinTriangle(
				trgle->A,trgle->B,trgle->C, p))
			{
				
				cout << fixed <<"x=" << p->x <<"," << "y=" << p->y << ","<<"height=" <<p->z << endl;
				cout << endl;
				return true;
			}
		}
		tit++;
	}
	cout << fixed << "point: x = " << p->x <<", y = " << p->y << " is not in this area." << endl;
	cout << endl;
	return false;
}

bool HeightCalculator::PointinTriangle(Point* A, Point* B, Point* C,Point *P)
{
	Point v0 = *C - *A ;
	Point v1 = *B - *A ;
	Point v2 = *P - *A ;

	double dot00 = v0.Dot(v0) ;
	double dot01 = v0.Dot(v1) ;
	double dot02 = v0.Dot(v2) ;
	double dot11 = v1.Dot(v1) ;
	double dot12 = v1.Dot(v2) ;

	double inverDeno = 1 / (dot00 * dot11 - dot01 * dot01) ;

	double u = (dot11 * dot02 - dot01 * dot12) * inverDeno ;
	if (u < 0 || u > 1) // if u out of range, return directly
	{
		P->z = -1;
		return false ;
	}

	double v = (dot00 * dot12 - dot01 * dot02) * inverDeno ;
	if (v < 0 || v > 1) // if v out of range, return directly
	{
		P->z = -1;
		return false ;
	}

	if (u + v <= 1 )
	{
		P->z = A->z +  u * (C->z - A->z) + v * (B->z - A->z);
		return true;
	}
	P->z =-1;
	return false;
}

void split(string& s, string& delim,vector< string >&ret) 
{ 	
	size_t last = 0; 	
	size_t index=s.find_first_of(delim,last); 	
	while (index!=string::npos) 	
	{ 		
		ret.push_back(s.substr(last,index-last)); 		
		last=index+1; 		
		index=s.find_first_of(delim,last); 	
	} 	
	if (index-last>0) 	
	{ 		
		ret.push_back(s.substr(last,index-last)); 	
	} 
}
