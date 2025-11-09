#include<iostream>
#include<cstring>
using namespace std;

class A{
    char *name;
    public:
    A(const char *p)
    {
        name=new char[strlen(p)+1];
        strcpy(name,p);
    }
    virtual ~A()
    {
        cout<<"destructor A"<<endl;
        if(name) delete []name;
    }
    virtual void show()
    {
        cout<<name<<endl;
    }
    virtual void print()=0;

};
class B:public A{
    int age;
    public:
    B(const char* n, int a):A(n),age(a){}
    ~B()
    {
        cout<<"destructor B"<<endl;
    }
    void print()
    {
        cout<<age<<endl;
    }
    void show()
    {
        cout<<"B:";
        A::show();
        print();
    }
};
class C:public A{
    char gender;
    public:
    C(const char* n,char g) :A(n),gender(g) {}
    ~C()
    {
        cout<<"destructor C"<<endl;
    }
    void print()
    {
        cout<<gender<<endl;
    }
    void show()
    {
        cout<<"C:";
        A::show();
        print();
    }
};
class manager{
    A **array;
    int len;
    //const int size;     //输出的关键点，不能定义size
    public:
    manager(int k):len(0)
    {
        array=new A*[k];
        for (int i = 0; i <k; ++i) 
        {
            array[i] = nullptr;
        }
    }
    ~manager() 
    {
        int i=0,j=0;
        while(j<len)
        {
            if(array[i])
            {
                delete array[i];
                ++j;
            }
            ++i;
        }
        if(array)delete[] array;
    }
    void add(int index, const char* name, int age) {
        array[index] = new B(name, age);
        ++len;
    }

    void add(int index, const char* name, char gender) {
        array[index]=new C(name,gender);
        ++len;
    }
    void display()
    {
        int i=0,j=0;
       while(j<len)
        {
            if(array[i])
            {
                array[i]->show();
                ++j;
            }
            ++i;
        }
    }

};





int main(){
    manager m(4);
    m.add(0,"zhang",18);
    m.add(1,"wang",'F');
    m.add(3,"liu",'M');
    m.display();

    system("pause");
    return 0;
}