#include<iostream>
#include<cstring>
using namespace std;


class Test{
	public:
		static int static_t;
		char char_t;
		int int_t;
		float float_t;
		char *char_ptr;
		int &ref_t;
    
		//构造函数与析构函数 
	    Test(char c,int i,float s,const char*cc,int &r):char_t(c),int_t(i),float_t(s),ref_t(r)
		{	
			char_ptr=new char[strlen(cc)+1];
			strcpy(char_ptr,cc);
			//注：不能在构造函数体内初始化引用 
		}	
		Test(const Test &t):char_t(t.char_t),int_t(t.int_t),float_t(t.float_t),ref_t(t.ref_t)
		{
			char_ptr=new char[strlen(t.char_ptr)+1];
			strcpy(char_ptr,t.char_ptr);
			
		}
		~Test()
		{
			if(char_ptr) delete[]char_ptr;
		}
		
		//静态成员函数与非静态成员函数
		static void static_func(){}
		void nostatic_func(){}
		void show()const
		{
	        cout  << " 静态成员:" <<Test::static_t << " 地址:" << &Test::static_t << " 存储空间:"<< sizeof(Test::static_t) <<"bytes"<<endl
	             << " 非静态成员:"<<endl
				 << " 字符:" << char_t << " 地址:" << (void*)&char_t <<" 存储空间:"<<sizeof(char_t) <<"bytes"<< endl
			     << " 整型:" << int_t << " 地址:" << &int_t <<" 存储空间:"<<sizeof(int_t) <<"bytes"<< endl
	             << " 浮点型:" <<float_t<< " 地址:" << &float_t<<" 存储空间:"<<sizeof(float_t) <<"bytes" << endl
	             << " 字符值:" << char_ptr<< " 地址:" <<(void*)char_ptr<<" 存储空间:"<<sizeof(char_ptr) <<"bytes"<< endl
	             << " 引用:" << ref_t << " 地址:" << &ref_t <<" 存储空间:"<<sizeof(ref_t) <<"bytes"<< endl;
		}
}; 

int Test::static_t=99; 

//2.定义多种对象并分析地址
//全局对象 
    int global_ref1=10,global_ref2=20;  
    Test global_t1('a',1,1.5F,"this is global_t1",global_ref1); 
    Test global_t2('b',2,2.5F,"this is global_t2",global_ref2);

//外部函数func定义局部对象 
void func(int i)
{
	Test local_func_t1('c',3,3.5F,"this is local_func_t1",global_ref1);
	Test local_func_t2('d',4,4.5F,"this is local_func_t2",global_ref2); 
	if(i==2)
	{
		cout<<"func局部对象1地址: "<<&local_func_t1<<endl;
		cout<<"func局部对象2地址: "<<&local_func_t2<<endl;
	}
	else if(i==3)
	{
		cout<<"func局部成员信息:"<<endl;
		local_func_t1.show();
		cout<<endl;
	    local_func_t2.show();
	}
 } 

 union FuncPtrConverter {
    void (Test::*memberFunc)();
    void *voidPtr;
};
 union FuncPtrConverter1 {
	void (Test::*memberFunc)() const;
	void *voidPtr;
};




int main(){
	//main定义局部对象 
	int local_ref1=100,local_ref2=200;
	Test local_t1('e',5,5.5F,"this is local_t1",local_ref1);
	Test local_t2('f',6,6.5F,"this is local_t2",local_ref2);
	
	//动态创建对象
    Test *dynamic_t1=new Test('g',7,7.5F,"dynamic_t1",global_ref1);
	Test *dynamic_t2=new Test('h',8,8.5F,"dynamic_t2",global_ref2);
	
	//2.输出各类对象地址 
	cout<<"2.定义多种对象并分析地址"<<endl;
	
	cout<<"全局对象1地址: "<<&global_t1<<endl;
    cout<<"全局对象2地址: "<<&global_t2<<endl;
	cout<<"main局部对象1地址: "<<&local_t1<<endl;
    cout<<"main局部对象2地址: "<<&local_t2<<endl;
	func(2);
    cout<<"动态对象1地址: "<<&dynamic_t1<<endl;
    cout<<"动态对象2地址: "<<&dynamic_t2<<endl;
	 
	cout<<endl<<endl;
	cout<<"3.输出对象中成员的值/地址/存储空间大小 "<<endl;
	// 输出静态成员信息
  
	cout<<"全局成员信息:"<<endl;
	global_t1.show();
	cout<<endl;
    global_t2.show();
    cout<<endl<<endl;
	cout<<"main局部成员信息:"<<endl;
	local_t1.show();
	cout<<endl;
    local_t2.show();
    cout<<endl<<endl;
	func(3);
	cout<<endl<<endl;
	cout<<"动态成员信息:"<<endl;
	dynamic_t1->show();
	cout<<endl;
    dynamic_t2->show();
    cout<<endl<<endl;
   
	//5.输出成员函数地址/外部函数地址 
	cout<<"5.输出成员函数地址/外部函数地址 "<<endl;
    cout << "静态成员函数地址: " << reinterpret_cast<void*>(Test::static_func) << endl;
     
	FuncPtrConverter converter;
    converter.memberFunc = &Test::nostatic_func;
    cout << "非静态成员函数地址: " << converter.voidPtr << endl;

	FuncPtrConverter1 converter1;
    converter1.memberFunc = &Test::show;
    cout << "show函数地址: " << converter1.voidPtr << endl;
	
	//cout << "非静态成员函数地址: " << reinterpret_cast<void*>(&Test::nostatic_func) << endl;
	//cout << "show函数地址: " << reinterpret_cast<void*>(&Test::show) << endl;
	cout << "外部函数地址: " << reinterpret_cast<void*>(func) << endl;
    cout << "主函数main地址: " << reinterpret_cast<void*>(main) << endl;	
	
		
	
	if(dynamic_t1)delete dynamic_t1;
	if(dynamic_t2)delete dynamic_t2;
		
    system("pause");
	return 0; 
}
 