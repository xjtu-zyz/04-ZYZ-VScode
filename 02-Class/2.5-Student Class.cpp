#include<iostream>
#include<cstring>
using namespace std;

class student{
    char *name;
    float g1,g2;
public:
    student(const char* a,float b,float c)
    {
        name = new char[strlen(a)+1];
        strcpy(name,a);
        g1=b;
        g2=c;
        cout<<"construct"<<" "<<name<<endl;
    }
    student(const student &s)
    {
        name = new char[strlen(s.name)+2];
        strcpy(name,s.name);
        name[strlen(s.name)]='u';
        name[strlen(s.name)+1]='\0';
        g1=s.g1-10.0;
        g2=s.g2-10.0;
        cout<<"copy"<<" "<<name<<endl;
    }
    ~student()
    {
        cout<<"destruct"<<" "<<name<<endl;
        if(name) delete []name;
    }


    float getg1(){return g1;}
    float getg2(){return g2;}
    student* getad(){return this;}

};


int main(){
    student a("li",90.0,80.0);
    student *st=new student[2]{student("zhang",80.0,70.0),student("wang",90.0,80.0)};

    cout<<(st[0].getg1()+st[0].getg2())/2<<" "<<(st[1].getg1()+st[1].getg2())/2<<endl;
    if(st) delete[]st;

    student b(a);
    cout<<&b-b.getad()<<endl;

    
    //system("pause");
    return 0;
}

