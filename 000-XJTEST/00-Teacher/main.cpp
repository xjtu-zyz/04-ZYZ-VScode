#include <iostream>
#include <vector>
#include <memory>
#include "teacher.h"

using namespace std;

int main()
{
    int nCount = 0;
    cout << "请输入教师人数：";
    cin >> nCount;

    vector<CTeacher *> lstTeacher;

    for (int i = 0; i < nCount; ++i)
    {
        int nTitle = 0;
        int nHours = 0;

        cout << "请输入第" << i + 1 << "位教师的职称（1-教授，2-副教授，3-讲师）：";
        cin >> nTitle;
        cout << "请输入该教师的月课时数：";
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

    cout << "\n各位教师的月工资如下：" << endl;
    for (int i = 0; i < lstTeacher.size(); ++i)
    {
        cout << "第" << i + 1 << "位教师工资: " << lstTeacher[i]->GetSalary() << " 元" << endl;
    }

    for (int i = 0; i < lstTeacher.size(); ++i)
    {
        delete lstTeacher[i];
    }
    system("pause");
    return 0;
}
