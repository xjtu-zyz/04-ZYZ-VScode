#include<iostream>
using namespace std;

class Base{
	int x;
	public:
		Base(int a)
		{
			x=a;
		}
		void show()
		{
			cout<<"x="<<x<<endl;
		}	
};
class derived:public Base{
	int y;
	public:
		derived(int a,int b):Base(a)
		{
			y=b;
		}
		void show()
		{
			Base::show();     
			cout<<"y="<<y<<endl;
		}
	
};

void func1(Base &a)
{
	a.show();         //输出x值 
	
}
void func2(derived &b)
{
	b.show();         //输出x和y值 
}

int main(){
	
	derived a(10,20);  
	//初始化a,调用基类Base的构造函数,使x=10;
	//然后调用派生类的构造函数,使y=20 
	a.show();
	//因为a属于derived派生类,调用派生类的show函数,输出x=10 y=20 
	Base *b=&a;
	derived *d=&a;
	b->show();
	d->show();
	//指针b属于基类Base,相当于将a转化为基类对象,调用基类的show函数,输出x=10 
	//指针d属于派生类derived,调用派生类的show函数,输出x=10 y=20 
	func1(a);
	func2(a);
	//func1的形参属于基类Base, 当于将a转化为基类对象,调用基类的show函数,输出x=10 
	//func2的形参属于派生类derived,调用派生类的show函数,输出x=10 y=20  
	
	//system("pause");
	return 0;
} 