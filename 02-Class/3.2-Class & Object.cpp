/*
1������һ���࣬�����о�̬���ݳ�Ա���������ͷǾ�̬���ݳ�Ա�����ַ�ָ�룩��
   �����������ã���ѡ������̬�ͷǾ�̬��Ա������������ռ�Ĺ��캯����������������
2������ȫ�ֶ���main�����оֲ�������һ����main���õ��ⲿ����func�ж���ֲ����󣨿������βΣ���
   main�����ж�̬��������ÿ�ֶ�������2�����۲졢�������ֶ����ַ��
3����������и�����̬��Ǿ�̬���ݳ�Ա��ֵ����ַ������Ĵ洢�ռ��С����Ϣ��
   �ɴ�������ı��ʡ���̬���ݳ�Ա�Ǳ��������һ�ݿ��������⡣���⣬Ӧ�۲��������
4������ѡ����������ÿ���ֽڣ��Խ�ʾ���õ�ʵ�ַ�����
5�������������ֶ��������̬���Ǿ�̬��Ա������ַ���Լ�main��func���ⲿ�����ĵ�ַ��
   ��������Ҫ����ú����������������������档 
*/

#include <iostream>
#include <cstring>

using namespace std;

int global_ref1 = 100, global_ref2 = 200;  // �������ó�Ա��ʼ��

class MyClass {
public:
    static int static_member;         // ��̬��Ա
    int int_member;                  // ���ͳ�Ա
    char* char_ptr;                  // �ַ�ָ��
    int& ref_member;                 // ���ó�Ա
    double double_member;            // ���ڹ۲��ڴ����
    
    // ���캯��
    MyClass(int i, const char* s, int& r, double d)
        : ref_member(r) {            // ��ʼ�����ó�Աc
        int_member = i;
        char_ptr = new char[strlen(s)+1];
        strcpy(char_ptr, s);
        double_member = d;
    }

    // �������캯���������
    MyClass(const MyClass& other)
        : ref_member(other.ref_member) {
        int_member = other.int_member;
        char_ptr = new char[strlen(other.char_ptr)+1];
        strcpy(char_ptr, other.char_ptr);
        double_member = other.double_member;
    }

    // ��������
    ~MyClass() {
        delete[] char_ptr;
    }

    // ��̬��Ա����
    static void static_func() {}

    // �Ǿ�̬��Ա����
    void non_static_func() {}

    // ��ʾ������Ϣ
    void show() const {
        cout << " �Ǿ�̬��Ա: " << endl
             << "   int_member: " << int_member 
             << " ��ַ: " << &int_member << endl
             << "   char_ptr: " << static_cast<void*>(char_ptr)
             << " ����: " << char_ptr << endl
             << "   ref_member: " << ref_member 
             << " ��ַ: " << &ref_member << endl
             << "   double_member: " << double_member
             << " ��ַ: " << &double_member << endl;
    }

    // ��ʾ�����ڴ沼��
    void show_memory() const {
        const unsigned char* p = reinterpret_cast<const unsigned char*>(this);
        cout << " �����ڴ� (" << sizeof(*this) << " �ֽ�): ";
        for (size_t i = 0; i < sizeof(*this); ++i) {
            printf("%02x ", p[i]);
        }
        cout << endl;
    }
};

// ��ʼ����̬��Ա
int MyClass::static_member = 999;

// ȫ�ֶ���
MyClass global1(1, "global1", global_ref1, 1.1);
MyClass global2(2, "global2", global_ref2, 2.2);

// �ⲿ����
void func(MyClass param) {
    MyClass local_in_func(5, "func_local", global_ref1, 5.5);
    cout << "\nfunc�оֲ�����:" << endl;
    cout << "��ַ: " << &local_in_func << endl;
    local_in_func.show();
}

int main() {
    // �ֲ�����
    int local_ref = 300;
    MyClass local1(3, "local1", local_ref, 3.3);
    MyClass local2(4, "local2", local_ref, 4.4);

    // ��̬����
    MyClass* dynamic1 = new MyClass(5, "dynamic1", global_ref1, 5.5);
    MyClass* dynamic2 = new MyClass(6, "dynamic2", global_ref2, 6.6);

    // �����̬��Ա��Ϣ
    cout << "��̬��Ա: " << MyClass::static_member 
         << " ��ַ: " << &MyClass::static_member << endl;

    // ���ȫ�ֶ�����Ϣ
    cout << "\nȫ�ֶ���1��ַ: " << &global1 << endl;
    global1.show();
    cout << "\nȫ�ֶ���2��ַ: " << &global2 << endl;
    global2.show();

    // ����ֲ�������Ϣ
    cout << "\n�ֲ�����1��ַ: " << &local1 << endl;
    local1.show();
    cout << "\n�ֲ�����2��ַ: " << &local2 << endl;
    local2.show();

    // �����̬������Ϣ
    cout << "\n��̬����1��ַ: " << dynamic1 << endl;
    dynamic1->show();
    cout << "\n��̬����2��ַ: " << dynamic2 << endl;
    dynamic2->show();

    // �����ⲿ����
    func(local1);

    // ���������ַ
    cout << "\n������ַ:" << endl;
    cout << "main:    " << reinterpret_cast<void*>(main) << endl
         << "func:    " << reinterpret_cast<void*>(func) << endl
         << "static_func:  " << reinterpret_cast<void*>(MyClass::static_func) << endl
         << "non_static_func: " 
         <<(void*)(&MyClass::non_static_func) << endl;

    // ����ڴ沼��
    cout << "\n�ڴ沼��ʾ��:" << endl;
    local1.show_memory();

    delete dynamic1;
    delete dynamic2;
    system("pause");
    return 0;
}