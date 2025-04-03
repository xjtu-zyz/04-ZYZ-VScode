#include<iostream>
#include<cstring>
using namespace std;

class String{
    char* array;
    public:
    String(const char* p)
    {
        array=new char[strlen(p)+1];
        strcpy(array,p);
    }
    ~String()
    {
        if(array)delete []array;
    }
     
    char& operator[](int index)
    {
        if(index<0 || index >=strlen(array))
        {   throw "Index out of range.";//throw��C++���쳣������ƣ�throw������Ը�һ���ַ�������ʾ������Ϣ
            /*
            cout<<"Index out of range."<<endl;
            static char dummy = '\0';
            return dummy;
            */
            //ע�⣡����Ҫ����һ���޹ص��ַ�����
        }
        else return array[index];
    }//����ֵ������Ҫ�Ǳ�������Ҫ�ı�ԭʼֵ�����Ҫ��������

    friend ostream& operator<<(ostream &output,const String &a)
    {
        output<<a.array;
        return output;
    } //String<<ʹ����Ԫ����������ostream����
 
    operator int()
    {
        return strlen(array);
    } //String+intʹ�ö����ǿ��ת����������+���أ�

    String& operator+(String &a)
    { //�β���Ҫ�����ã�����Ҫ�ÿ������죬δ���壬����ǳ����
      //�˴����Բ�дconst
        char* p=new char[strlen(array)+strlen(a.array)+1];
        strcpy(p,array);
        strcat(p,a.array);
        if(array)delete []array;
        array=p;
        return *this;
    }  //String+Stringʹ�ó�Ա���������޸Ķ�����ʹ��*this��������
    
	String operator+(const char*b)
    { //��д����ʱ����ǿ��ת��ab��string2���������ֲ������ж�����
      //�˴���Ҫдconst��������warning��ǿ��ת��
      
        char* p=new char[strlen(array)+strlen(b)+1];
        strcpy(p,array);
        strcat(p,b);
        //return p;//ע�⣡�����ڴ�й©�����ص���ָ�룬��������Ҫ�ͷ��ڴ�
        //return String(p);//ע�⣡�����ڴ�й©�����ص��Ƕ��󣬵�������Ҫ�ͷ��ڴ�
        String tmp(p);    //����ֲ����󣬻��ں�������������
        if(p)delete []p;
        return tmp;
        
    }  //String+"ab"ʹ�ó�Ա���������޸Ķ����򷵻ض���Ŀ���
/*  
    ��������String+"ab"���޸Ķ��󣬷����ַ���
    char* operator+(const char*b)
    {
        char* p=new char[strlen(array)+strlen(b)+1];
        strcpy(p,array);
        strcat(p,b);
        String tmp(p);    //����ֲ����󣬻��ں�������������
        if(p)delete []p;
        return tmp;  
    }  
    ��������String+"ab"��ab����ת���������ַ���
    char* operator+(const String &a)
    { //�˴�дconst����ǰһ��String+String����
      //����������ʱ���󡢾ֲ�����Ϊconst����
        char* p=new char[strlen(array)+strlen(a.array)+1];
        strcpy(p,array);
        strcat(p,a.array);
        String tmp(p);    //����ֲ����󣬻��ں�������������
        if(p)delete []p;
        return tmp;
    }  
*/
 
 
};








int main(){
    String 
    string1("mystring"),string2("yourstring"),string3("herstring");
    cout<<string1<<endl;
  try{
        string1[7]='n';
        cout<<string1<<endl;
        string1[8]='n';
    }  
    catch(const char* e)
    {
        cout<<e<<endl;
    }//�쳣���������쳣�����������Ϣ
    cout<<string1+string2+string3<<endl;
    cout<<string1<<endl;
    cout<<string2+"ab"<<endl;
    cout<<string2<<endl;
    cout<<string3+3<<endl;
    cout<<string3<<endl;
    system("pause");
    return 0;
}