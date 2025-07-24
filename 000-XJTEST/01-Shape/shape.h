#ifndef _SHAPE_H_
#define _SHAPE_H

#include<string>
#include<iostream>
#include<cmath>
using namespace std;

class CShape{
public:
    CShape(double dX,double dY,const string &sColor)
        :m_dX(dX),m_dY(dY),m_sColor(sColor){}
    virtual ~CShape(){}

    void SetPosition(double dX,double dY)
    {
        m_dX=dX;
        m_dY=dY;
    }
    void SetColor(const string &sColor)
    {
        m_sColor=sColor;
    }
    //virtual void show()=0; //定义纯虚函数show()
    virtual double GetArea()=0;     //面积
    virtual double GetPerimeter()=0;    //周长
    virtual void PrintInfo()=0;

protected:
    double m_dX,m_dY;
    string m_sColor;
};

class CCircle:public CShape{
public:
    CCircle(double dX,double dY,const string &sColor,double dRadius)
        :CShape(dX,dY,sColor),m_dRadius(dRadius){}
    virtual ~CCircle(){}

    void SetRadius(double dRadius)
    {
        m_dRadius=dRadius;
    }

    //virtual void show()=0; //定义纯虚函数show()
    virtual double GetArea()
    {
        return 3.1415926*pow(m_dRadius,2);
    }
    virtual double GetPerimeter() 
    {
        return 2*3.1415926*m_dRadius;
    }
    virtual void PrintInfo()
    {
        cout << "图元类型：圆" << endl;
        cout << "位置坐标：(" << m_dX << ", " << m_dY << ")" << endl;
        cout << "颜色：" << m_sColor << endl;
        cout << "半径：" << m_dRadius << endl;
        cout << "周长：" << GetPerimeter() << endl;
        cout << "面积：" << GetArea() << endl;
        cout << "-------------------------" << endl;
    }
    
private:
    double m_dRadius;
};

class CRect:public CShape{
public:
    CRect(double dX,double dY,const string &sColor,double dWidth,double dHeight)
        :CShape(dX,dY,sColor),m_dWidth(dWidth),m_dHeight(dHeight){}
    virtual ~CRect(){}

    void SetSize(double dwidth,double dHeight)
    {
        m_dWidth=dwidth;
        m_dHeight=dHeight;
    }

    //virtual void show()=0; //定义纯虚函数show()
    virtual double GetArea()
    {
        return m_dHeight*m_dWidth;
    }
    virtual double GetPerimeter() 
    {
        return 2*(m_dHeight+m_dWidth);
    }
    virtual void PrintInfo()
    {
        cout << "图元类型：矩形" << endl;
        cout << "位置坐标：(" << m_dX << ", " << m_dY << ")" << endl;
        cout << "颜色：" << m_sColor << endl;
        cout << "宽度：" << m_dWidth << "，高度：" << m_dHeight << endl;
        cout << "周长：" << GetPerimeter() << endl;
        cout << "面积：" << GetArea() << endl;
        cout << "-------------------------" << endl;
    }
    
private:
    double m_dWidth,m_dHeight;
};
#endif