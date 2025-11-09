#include<iostream>
#include<cstring>
using namespace std;

class student{
private:
   int gnormal,gfinal;
   static float proportion;
   //需要与全部成员共享，静态变量
public:
    student(): gnormal(0), gfinal(0) {}               //不能省略，因为teacher中的new student[num]需要调用默认构造函数
    student(int a,int b): gnormal(a), gfinal(b) {}    //初始化列表

    static void setProp()
    {
        cin>>proportion;  
    }
    void compScore()
    {
        cout<<gnormal*proportion+gfinal*(1-proportion)<<" ";
    }
};

class teacher{
private:
    student* pStu;
public:
    teacher(int num)
    {
        pStu=new student[num];
    }
    ~teacher()
    {
        if(pStu) delete []pStu;
    }
    void assign(int n)
    {
        int a,b;
        cin>>a>>b;
        pStu[n]=student(a,b);
    }
    void show(int n)
    {
        pStu[n].compScore();
    }
};

float student::proportion=0.4;
int main(){
    student::setProp();
    int n;
    cin>>n;
    teacher stu(n);
    
    for(int i=0;i<n;i++)
    {
        stu.assign(i);
    }
    for(int i=0;i<n;i++)
    {
        stu.show(i);
    }



    //system("pause");
    return 0;
}