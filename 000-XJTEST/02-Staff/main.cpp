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

        cout << "�����뼼����Ա������";
        cin >> sName;
        pTech->SetName(sName);

        cout << "�����빤��Сʱ����ʱн��";
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

        cout << "������������Ա������";
        cin >> sName;
        pSales->SetName(sName);

        cout << "���������۶����ɱ�����";
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

        cout << "�����뾭��������";
        cin >> sName;
        pManager->SetName(sName);

        cout << "������̶���н��";
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

        cout << "���������۾���������";
        cin >> sName;
        pSM->SetName(sName);

        cout << "���������۶����ɱ����͹̶���н��";
        cin>> dSalesMoney >> dProportion>> dFixedSalary;
        pSM->SetSalesMoney(dSalesMoney);    
        pSM->SetProportion(dProportion);
        pSM->SetFixedSalary(dFixedSalary);
        pSM->ComputeSalary();

        lstEmployee.push_back(pSM);
    }

    cout << "\n--- ����Ա����Ϣ���� ---" << endl;
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