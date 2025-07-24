#include <iostream>
#include <vector>
#include <memory>
#include "teacher.h"

using namespace std;

int main()
{
    int nCount = 0;
    cout << "�������ʦ������";
    cin >> nCount;

    vector<CTeacher *> lstTeacher;

    for (int i = 0; i < nCount; ++i)
    {
        int nTitle = 0;
        int nHours = 0;

        cout << "�������" << i + 1 << "λ��ʦ��ְ�ƣ�1-���ڣ�2-�����ڣ�3-��ʦ����";
        cin >> nTitle;
        cout << "������ý�ʦ���¿�ʱ����";
        cin >> nHours;

        CTeacher *pTeacher = nullptr;
        if (nTitle == 1)
        {
            pTeacher = new CProfessor(nHours);
        }
        else if (nTitle == 2)
        {
            pTeacher = new CAssociateProfessor(nHours);
        }
        else if (nTitle == 3)
        {
            pTeacher = new CLecturer(nHours);
        }

        if (pTeacher != nullptr)
        {
            lstTeacher.push_back(pTeacher);
        }
    }

    cout << "\n��λ��ʦ���¹������£�" << endl;
    for (int i = 0; i < lstTeacher.size(); ++i)
    {
        cout << "��" << i + 1 << "λ��ʦ����: " << lstTeacher[i]->GetSalary() << " Ԫ" << endl;
    }

    for (int i = 0; i < lstTeacher.size(); ++i)
    {
        delete lstTeacher[i];
    }
    system("pause");
    return 0;
}
