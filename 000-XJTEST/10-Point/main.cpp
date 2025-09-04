#include <iostream>
#include "heightcalculator.h"

int main()
{
	HeightCalculator cal;
	cal.LoadTrigle();
	cal.LoadPoints();
	cal.CalculatePointsHeight();
	cout<<"put in y to get start, q to quit" << endl;
	while(1)
	{
		if (getchar() == 'q')
		{
			break;
		}
		double x=441892.84, y=4659984.96;
		cout<<"input x and y, x goes first" << endl;
		cin>>x>>y;
		Point p(x,y,-1);
		//load traingle file to buff
		cal.LocatePoint(&p);
	}
	
	return 0;
}
