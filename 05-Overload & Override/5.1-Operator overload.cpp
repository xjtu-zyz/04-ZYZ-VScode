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
        {   throw "Index out of range.";//throw是C++的异常处理机制，throw后面可以跟一个字符串，表示错误信息
            /*
            cout<<"Index out of range."<<endl;
            static char dummy = '\0';
            return dummy;
            */
            //注意！！需要返回一个无关的字符变量
        }
        else return array[index];
    }//作左值，必须要是变量，需要改变原始值，因此要返回引用

    friend ostream& operator<<(ostream &output,const String &a)
    {
        output<<a.array;
        return output;
    } //String<<使用友元函数，返回ostream引用
 
    operator int()
    {
        return strlen(array);
    } //String+int使用对象的强制转换（不能用+重载）

    String& operator+(String &a)
    { //形参需要用引用，否则要用拷贝构造，未定义，出现浅拷贝
      //此处可以不写const
        char* p=new char[strlen(array)+strlen(a.array)+1];
        strcpy(p,array);
        strcat(p,a.array);
        if(array)delete []array;
        array=p;
        return *this;
    }  //String+String使用成员函数，会修改对象，则使用*this返回引用
    
	String operator+(const char*b)
    { //不写函数时，会强制转换ab或string2，出现两种操作，有二义性
      //此处需要写const，否则有warning：强制转换
      
        char* p=new char[strlen(array)+strlen(b)+1];
        strcpy(p,array);
        strcat(p,b);
        //return p;//注意！！有内存泄漏，返回的是指针，调用者需要释放内存
        //return String(p);//注意！！有内存泄漏，返回的是对象，调用者需要释放内存
        String tmp(p);    //定义局部对象，会在函数结束后析构
        if(p)delete []p;
        return tmp;
        
    }  //String+"ab"使用成员函数，不修改对象，则返回对象的拷贝
/*  
    方法二：String+"ab"不修改对象，返回字符串
    char* operator+(const char*b)
    {
        char* p=new char[strlen(array)+strlen(b)+1];
        strcpy(p,array);
        strcat(p,b);
        String tmp(p);    //定义局部对象，会在函数结束后析构
        if(p)delete []p;
        return tmp;  
    }  
    方法三：String+"ab"将ab类型转换，返回字符串
    char* operator+(const String &a)
    { //此处写const，与前一个String+String区分
      //匿名对象、临时对象、局部对象都为const类型
        char* p=new char[strlen(array)+strlen(a.array)+1];
        strcpy(p,array);
        strcat(p,a.array);
        String tmp(p);    //定义局部对象，会在函数结束后析构
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
    }//异常处理，捕获异常，输出错误信息
    cout<<string1+string2+string3<<endl;
    cout<<string1<<endl;
    cout<<string2+"ab"<<endl;
    cout<<string2<<endl;
    cout<<string3+3<<endl;
    cout<<string3<<endl;
    system("pause");
    return 0;
}