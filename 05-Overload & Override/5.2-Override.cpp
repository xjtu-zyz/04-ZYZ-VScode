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
    void show()
    {
        cout<<name<<endl;
    }
    virtual void print()=0;
    virtual void display()=0;

};
class B:public A{
    int age;
    public:
    B(const char* n, int a) : A(n), age(a) {}
    ~B()
    {
        cout<<"destructor B"<<endl;
    }
    void print()
    {
        cout<<age<<endl;
    }
    void display()
    {
        cout<<"B:";
        show();
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
    void display()
    {
        cout<<"C:";
        show();
        print();
    }
};
class manager{
    A **array;
    int len;
    const int size;     //输出的关键点
    public:
    manager(int size):size(size),len(0)
    {
        array=new A*[size];
        for (int i = 0; i <size; ++i) 
        {
            array[i] = nullptr;
        }
    }
    ~manager() 
    {
        for (int i = 0; i < size; ++i) {
            if(array[i])delete array[i];
        }
        if(array)delete[] array;
    }
    void add(int index, const char* name, int age) {
        if (index < 0 || index >= size) return;
        if (array[index]) {
            delete array[index];
            --len;
        }
        array[index] = new B(name, age);
        ++len;
    }

    void add(int index, const char* name, char gender) {
        if (index < 0 || index >= size) return;
        if (array[index]) {
            delete array[index];
            --len;
        }
        array[index] = new C(name, gender);
        ++len;
    }
    void display()
    {
        for(int i=0; i<size; ++i)
        {
            if (array[i]) array[i]->display();
        }
            
    }

};





int main(){
    manager m(4);
    m.add(0,"zhang",18);
    m.add(1,"wang",'F');
    m.add(3,"liu",'M');
    m.display();

    return 0;
}