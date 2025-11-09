#include<iostream>
#include<cstring>
using namespace std;

class A{
    int a;
    public:
    A(int x=0):a(x){}
    
    virtual A& operator+(int x)
    {
        a+=x;
        return *this; //返回引用，避免拷贝构造
    }
    
    virtual void show()
    {
        cout<<"a="<<a<<endl;
    }
    
};
class B:public A{
    int b;
    public:
    B(int x=0,int y=0):A(x),b(y){}

    B& operator+(int y)
    {
        b+=y;
        return *this;
    }
    void show()
    {
        A::show();
        cout<<"b="<<b<<endl;
    }

};


void add(A& a, int x)
{
    a = a + x;
}

int main(){
    A myA,*p;
    B myB(2,3);
    add(myA,3); p=&myA;p->show();
    add(myB,3); p=&myB;p->show();
    return 0;
}