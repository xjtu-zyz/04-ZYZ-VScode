#include"shape.h"
#include<vector>

int main()
{
    vector<CShape *>lstShape;

    CShape *pCircle=new CCircle(10,20.5,"red",5);
    lstShape.push_back(pCircle);

    CShape *pRect=new CRect(30,40.5,"blue",2.0,4.0);
    lstShape.push_back(pRect);
    
    for(int i=0;i<lstShape.size();i++)
    {
        lstShape[i]->PrintInfo();
        delete lstShape[i];
    }
    
    system("pause");
    return 0;
}