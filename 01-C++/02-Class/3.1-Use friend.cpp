#include<iostream>
#include<cstring>
using namespace std;

class student;
class teacher{
private:
    student* pStu;
public:
    teacher(int num);
    ~teacher();
    void assign(int n);
    void show(int n);
};

class student{
private:
    int gnormal,gfinal;
    static float proportion;
    //需要与全部成员共享，静态变量
public:
    friend void teacher::assign(int n);
    static void setProp()
    {
        cin>>proportion;  
    }
    void compScore()
    {
        cout<<gnormal*proportion+gfinal*(1-proportion)<<" ";
    }
};

    teacher::teacher(int num)
    {
        pStu=new student[num];
    }
    teacher::~teacher()
    {
        if(pStu) delete []pStu;
    }
    void teacher::assign(int n)
    {
        cin>>pStu[n].gnormal>>pStu[n].gfinal;
    }
    void teacher::show(int n)
    {
        pStu[n].compScore();
    }

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