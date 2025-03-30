#include<iostream>
#include<cstring>
using namespace std;

class Base{
    private:
    char *name;
    int age;
    //法二：解决重复显示：bool flag;
    public:
    Base(const char*n,int a){
        //flag=false;
        age=a;
        name=new char[strlen(n)+1];
        strcpy(name,n);
        cout<<"Base constructor"<<endl;
    }
    ~Base(){
        if(name) delete []name;
        cout<<"Base destructor"<<endl;
    }
    void show(){
        /*if(!s)
        {
            cout<<name<<" "<<age<<" ";
        }*/
        cout<<name<<" "<<age<<" ";
    }
};

class Leader:virtual protected Base{
    private:
    char pos[10];   //position
    public:
    Leader(const char*n,const char*p,int a):Base(n,a){
        strcpy(pos,p);
        cout<<"Leader constructor"<<endl;
    }
    ~Leader(){
        cout<<"Leader destructor"<<endl;
    }
    void show(){
        /* if(!flag)
        {
            Base::show();
            flag=true;
        }*/
        Base::show();
        cout<<pos<<" ";
    }
};

//两个派生类中可有其中一个使用private派生
class Engineer:virtual protected Base{
    private:
    char major[10];
    public:
    Engineer(const char*n,const char*m,int a):Base(n,a){
        strcpy(major,m);
        cout<<"Engineer constructor"<<endl;
    }
    ~Engineer(){
        cout<<"Engineer destructor"<<endl;
    }
    const char* getMajor() const {
        return major;
    }
    //为了符合第三题的show函数构造要求，取出major的值
    void show(){
        Base::show();
        cout<<major<<" ";
    }

};

//可以都使用private派生
class Chairman:public Leader,public Engineer{
    public:
    Chairman(const char*n,const char*p,const char*m,int a):Base(n,a),Leader(n,p,a),Engineer(n,m,a){
        cout<<"Chairman constructor"<<endl;
    }
    ~Chairman(){
        cout<<"Chairman destructor"<<endl;
    }
    void show(){
        Leader::show();
        //Engineer::show();
        cout<<Engineer::getMajor()<<endl;
    }

};



int main(){
    Chairman a("Li","chair","computer",20);
    a.show();

    //system("pause");
    return 0;
}