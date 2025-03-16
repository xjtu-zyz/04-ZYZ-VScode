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
    
		//���캯������������ 
	    Test(char c,int i,float s,const char*cc,int &r):char_t(c),int_t(i),float_t(s),ref_t(r)
		{	
			char_ptr=new char[strlen(cc)+1];
			strcpy(char_ptr,cc);
			//ע�������ڹ��캯�����ڳ�ʼ������ 
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
		
		//��̬��Ա������Ǿ�̬��Ա����
		static void static_func(){}
		void nostatic_func(){}
		void show()const
		{
	        cout  << " ��̬��Ա:" <<Test::static_t << " ��ַ:" << &Test::static_t << " �洢�ռ�:"<< sizeof(Test::static_t) <<"bytes"<<endl
	             << " �Ǿ�̬��Ա:"<<endl
				 << " �ַ�:" << char_t << " ��ַ:" << (void*)&char_t <<" �洢�ռ�:"<<sizeof(char_t) <<"bytes"<< endl
			     << " ����:" << int_t << " ��ַ:" << &int_t <<" �洢�ռ�:"<<sizeof(int_t) <<"bytes"<< endl
	             << " ������:" <<float_t<< " ��ַ:" << &float_t<<" �洢�ռ�:"<<sizeof(float_t) <<"bytes" << endl
	             << " �ַ�ֵ:" << char_ptr<< " ��ַ:" <<(void*)char_ptr<<" �洢�ռ�:"<<sizeof(char_ptr) <<"bytes"<< endl
	             << " ����:" << ref_t << " ��ַ:" << &ref_t <<" �洢�ռ�:"<<sizeof(ref_t) <<"bytes"<< endl;
		}
}; 

int Test::static_t=99; 

//2.������ֶ��󲢷�����ַ
//ȫ�ֶ��� 
    int global_ref1=10,global_ref2=20;  
    Test global_t1('a',1,1.5F,"this is global_t1",global_ref1); 
    Test global_t2('b',2,2.5F,"this is global_t2",global_ref2);

//�ⲿ����func����ֲ����� 
void func(int i)
{
	Test local_func_t1('c',3,3.5F,"this is local_func_t1",global_ref1);
	Test local_func_t2('d',4,4.5F,"this is local_func_t2",global_ref2); 
	if(i==2)
	{
		cout<<"func�ֲ�����1��ַ: "<<&local_func_t1<<endl;
		cout<<"func�ֲ�����2��ַ: "<<&local_func_t2<<endl;
	}
	else if(i==3)
	{
		cout<<"func�ֲ���Ա��Ϣ:"<<endl;
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
	//main����ֲ����� 
	int local_ref1=100,local_ref2=200;
	Test local_t1('e',5,5.5F,"this is local_t1",local_ref1);
	Test local_t2('f',6,6.5F,"this is local_t2",local_ref2);
	
	//��̬��������
    Test *dynamic_t1=new Test('g',7,7.5F,"dynamic_t1",global_ref1);
	Test *dynamic_t2=new Test('h',8,8.5F,"dynamic_t2",global_ref2);
	
	//2.�����������ַ 
	cout<<"2.������ֶ��󲢷�����ַ"<<endl;
	
	cout<<"ȫ�ֶ���1��ַ: "<<&global_t1<<endl;
    cout<<"ȫ�ֶ���2��ַ: "<<&global_t2<<endl;
	cout<<"main�ֲ�����1��ַ: "<<&local_t1<<endl;
    cout<<"main�ֲ�����2��ַ: "<<&local_t2<<endl;
	func(2);
    cout<<"��̬����1��ַ: "<<&dynamic_t1<<endl;
    cout<<"��̬����2��ַ: "<<&dynamic_t2<<endl;
	 
	cout<<endl<<endl;
	cout<<"3.��������г�Ա��ֵ/��ַ/�洢�ռ��С "<<endl;
	// �����̬��Ա��Ϣ
  
	cout<<"ȫ�ֳ�Ա��Ϣ:"<<endl;
	global_t1.show();
	cout<<endl;
    global_t2.show();
    cout<<endl<<endl;
	cout<<"main�ֲ���Ա��Ϣ:"<<endl;
	local_t1.show();
	cout<<endl;
    local_t2.show();
    cout<<endl<<endl;
	func(3);
	cout<<endl<<endl;
	cout<<"��̬��Ա��Ϣ:"<<endl;
	dynamic_t1->show();
	cout<<endl;
    dynamic_t2->show();
    cout<<endl<<endl;
   
	//5.�����Ա������ַ/�ⲿ������ַ 
	cout<<"5.�����Ա������ַ/�ⲿ������ַ "<<endl;
    cout << "��̬��Ա������ַ: " << reinterpret_cast<void*>(Test::static_func) << endl;
     
	FuncPtrConverter converter;
    converter.memberFunc = &Test::nostatic_func;
    cout << "�Ǿ�̬��Ա������ַ: " << converter.voidPtr << endl;

	FuncPtrConverter1 converter1;
    converter1.memberFunc = &Test::show;
    cout << "show������ַ: " << converter1.voidPtr << endl;
	
	//cout << "�Ǿ�̬��Ա������ַ: " << reinterpret_cast<void*>(&Test::nostatic_func) << endl;
	//cout << "show������ַ: " << reinterpret_cast<void*>(&Test::show) << endl;
	cout << "�ⲿ������ַ: " << reinterpret_cast<void*>(func) << endl;
    cout << "������main��ַ: " << reinterpret_cast<void*>(main) << endl;	
	
		
	
	if(dynamic_t1)delete dynamic_t1;
	if(dynamic_t2)delete dynamic_t2;
		
    system("pause");
	return 0; 
}
 