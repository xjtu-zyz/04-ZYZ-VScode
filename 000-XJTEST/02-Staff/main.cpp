#include"staff.h"
#include<vector>
#include<iostream>

int main()
{
    vector<CEmployee *>lstEmployee;
    int nNum=1001;
    for(int i=0;i<2;i++)
    {
        CTechnician *pTech=new CTechnician(nNum++);
        string sName;
        int nHours;
        double dHourlyWage;

        cout << "请输入技术人员姓名：";
        cin >> sName;
        pTech->SetName(sName);

        cout << "请输入工作小时数和时薪：";
        cin>> nHours >> dHourlyWage;
        pTech->SetHours(nHours);    
        pTech->SetHourlyWage(dHourlyWage);
        pTech->ComputeSalary();

        lstEmployee.push_back(pTech);
    }

    for(int i=0;i<2;i++)
    {
        CSalesman *pSales=new CSalesman(nNum++);
        string sName;
        double dSalesMoney, dProportion;

        cout << "请输入销售人员姓名：";
        cin >> sName;
        pSales->SetName(sName);

        cout << "请输入销售额和提成比例：";
        cin>> dSalesMoney >> dProportion;
        pSales->SetSalesMoney(dSalesMoney);    
        pSales->SetProportion(dProportion);
        pSales->ComputeSalary();

        lstEmployee.push_back(pSales);
    }

    for(int i=0;i<2;i++)
    {
        CManager *pManager=new CManager(nNum++);
        string sName;
        double dFixedSalary;

        cout << "请输入经理姓名：";
        cin >> sName;
        pManager->SetName(sName);

        cout << "请输入固定月薪：";
        cin>> dFixedSalary;
        pManager->SetFixedSalary(dFixedSalary);
        pManager->ComputeSalary();

        lstEmployee.push_back(pManager);
    }
    
    for(int i=0;i<2;i++)
    {
        CSalesManager *pSM=new CSalesManager(nNum++);
        string sName;
        double dSalesMoney, dProportion,dFixedSalary;

        cout << "请输入销售经理姓名：";
        cin >> sName;
        pSM->SetName(sName);

        cout << "请输入销售额和提成比例和固定月薪：";
        cin>> dSalesMoney >> dProportion>> dFixedSalary;
        pSM->SetSalesMoney(dSalesMoney);    
        pSM->SetProportion(dProportion);
        pSM->SetFixedSalary(dFixedSalary);
        pSM->ComputeSalary();

        lstEmployee.push_back(pSM);
    }

    cout << "\n--- 所有员工信息如下 ---" << endl;
    for (int i = 0; i < lstEmployee.size(); ++i)
    {
        lstEmployee[i]->GetNum();
        lstEmployee[i]->GetName();
        lstEmployee[i]->GetSalary();
        delete lstEmployee[i];
    }

    system("pause");
    return 0;
}