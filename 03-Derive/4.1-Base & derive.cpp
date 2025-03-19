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
	a.show();         //���xֵ 
	
}
void func2(derived &b)
{
	b.show();         //���x��yֵ 
}

int main(){
	
	derived a(10,20);  
	//��ʼ��a,���û���Base�Ĺ��캯��,ʹx=10;
	//Ȼ�����������Ĺ��캯��,ʹy=20 
	a.show();
	//��Ϊa����derived������,�����������show����,���x=10 y=20 
	Base *b=&a;
	derived *d=&a;
	b->show();
	d->show();
	//ָ��b���ڻ���Base,�൱�ڽ�aת��Ϊ�������,���û����show����,���x=10 
	//ָ��d����������derived,�����������show����,���x=10 y=20 
	func1(a);
	func2(a);
	//func1���β����ڻ���Base, ���ڽ�aת��Ϊ�������,���û����show����,���x=10 
	//func2���β�����������derived,�����������show����,���x=10 y=20  
	
	//system("pause");
	return 0;
} 