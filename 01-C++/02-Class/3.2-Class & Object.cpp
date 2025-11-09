/*
1、定义一个类，其中有静态数据成员、各种类型非静态数据成员（含字符指针），
   甚至包括引用（可选），静态和非静态成员函数（含分配空间的构造函数、析构函数）。
2、定义全局对象、main函数中局部对象、另一个被main调用的外部函数func中定义局部对象（可以是形参）、
   main函数中动态创建对象，每种对象至少2个。观察、分析各种对象地址。
3、输出对象中各个静态与非静态数据成员的值、地址、对象的存储空间大小等信息。
   由此理解对象的本质、静态数据成员是本类对象共享一份拷贝等问题。此外，应观察对齐现象。
4、（可选）输出对象的每个字节，以揭示引用的实现方法。
5、对于上述各种对象，输出静态、非静态成员函数地址，以及main、func等外部函数的地址，
   并分析。要求采用合理方法，避免编译器提出警告。 
*/

#include <iostream>
#include <cstring>

using namespace std;

int global_ref1 = 100, global_ref2 = 200;  // 用于引用成员初始化

class MyClass {
public:
    static int static_member;         // 静态成员
    int int_member;                  // 整型成员
    char* char_ptr;                  // 字符指针
    int& ref_member;                 // 引用成员
    double double_member;            // 用于观察内存对齐
    
    // 构造函数
    MyClass(int i, const char* s, int& r, double d)
        : ref_member(r) {            // 初始化引用成员c
        int_member = i;
        char_ptr = new char[strlen(s)+1];
        strcpy(char_ptr, s);
        double_member = d;
    }

    // 拷贝构造函数（深拷贝）
    MyClass(const MyClass& other)
        : ref_member(other.ref_member) {
        int_member = other.int_member;
        char_ptr = new char[strlen(other.char_ptr)+1];
        strcpy(char_ptr, other.char_ptr);
        double_member = other.double_member;
    }

    // 析构函数
    ~MyClass() {
        delete[] char_ptr;
    }

    // 静态成员函数
    static void static_func() {}

    // 非静态成员函数
    void non_static_func() {}

    // 显示对象信息
    void show() const {
        cout << " 非静态成员: " << endl
             << "   int_member: " << int_member 
             << " 地址: " << &int_member << endl
             << "   char_ptr: " << static_cast<void*>(char_ptr)
             << " 内容: " << char_ptr << endl
             << "   ref_member: " << ref_member 
             << " 地址: " << &ref_member << endl
             << "   double_member: " << double_member
             << " 地址: " << &double_member << endl;
    }

    // 显示对象内存布局
    void show_memory() const {
        const unsigned char* p = reinterpret_cast<const unsigned char*>(this);
        cout << " 对象内存 (" << sizeof(*this) << " 字节): ";
        for (size_t i = 0; i < sizeof(*this); ++i) {
            printf("%02x ", p[i]);
        }
        cout << endl;
    }
};

// 初始化静态成员
int MyClass::static_member = 999;

// 全局对象
MyClass global1(1, "global1", global_ref1, 1.1);
MyClass global2(2, "global2", global_ref2, 2.2);

// 外部函数
void func(MyClass param) {
    MyClass local_in_func(5, "func_local", global_ref1, 5.5);
    cout << "\nfunc中局部对象:" << endl;
    cout << "地址: " << &local_in_func << endl;
    local_in_func.show();
}

int main() {
    // 局部对象
    int local_ref = 300;
    MyClass local1(3, "local1", local_ref, 3.3);
    MyClass local2(4, "local2", local_ref, 4.4);

    // 动态对象
    MyClass* dynamic1 = new MyClass(5, "dynamic1", global_ref1, 5.5);
    MyClass* dynamic2 = new MyClass(6, "dynamic2", global_ref2, 6.6);

    // 输出静态成员信息
    cout << "静态成员: " << MyClass::static_member 
         << " 地址: " << &MyClass::static_member << endl;

    // 输出全局对象信息
    cout << "\n全局对象1地址: " << &global1 << endl;
    global1.show();
    cout << "\n全局对象2地址: " << &global2 << endl;
    global2.show();

    // 输出局部对象信息
    cout << "\n局部对象1地址: " << &local1 << endl;
    local1.show();
    cout << "\n局部对象2地址: " << &local2 << endl;
    local2.show();

    // 输出动态对象信息
    cout << "\n动态对象1地址: " << dynamic1 << endl;
    dynamic1->show();
    cout << "\n动态对象2地址: " << dynamic2 << endl;
    dynamic2->show();

    // 调用外部函数
    func(local1);

    // 输出函数地址
    cout << "\n函数地址:" << endl;
    cout << "main:    " << reinterpret_cast<void*>(main) << endl
         << "func:    " << reinterpret_cast<void*>(func) << endl
         << "static_func:  " << reinterpret_cast<void*>(MyClass::static_func) << endl
         << "non_static_func: " 
         <<(void*)(&MyClass::non_static_func) << endl;

    // 输出内存布局
    cout << "\n内存布局示例:" << endl;
    local1.show_memory();

    delete dynamic1;
    delete dynamic2;
    system("pause");
    return 0;
}