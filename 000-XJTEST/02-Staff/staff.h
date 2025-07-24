#ifndef _STAFF_H_
#define _STAFF_H_

#include<string>
#include<iostream>
using namespace std;

class CEmployee{
public:
    //�����ĳ�ʼ��ͨ������¼��
    CEmployee(int nNum)
        :m_nNum(nNum),m_dSalary(0.0){}
    virtual ~CEmployee(){}

    void SetName(const string &sName)
    {
        m_sName=sName;
    }
    void GetName()
    {
        cout<<"Name:"<<m_sName<<endl;
    }
    void GetNum()
    {
        cout<<"Number:"<<m_nNum<<endl;
    }
    void GetSalary()
    {
        cout<<"Salary:"<<m_dSalary<<endl;
    }
    virtual void ComputeSalary()=0;

protected:
    int m_nNum;
    string m_sName;
    double m_dSalary;
};

//������Ա
class CTechnician:public CEmployee{
public:
    CTechnician(int nNum)
        :CEmployee(nNum),m_nHours(0),m_dHourlyWage(0.0){}
    void SetHours(int nHours)
    {
        m_nHours=nHours;
    }
    void SetHourlyWage(double dHourlyWage)
    {
        m_dHourlyWage=dHourlyWage;
    }

    virtual void ComputeSalary()
    {
        m_dSalary=m_dHourlyWage*m_nHours;
    }
private:
    int m_nHours;
    double m_dHourlyWage;
};

//����Ա
class CSalesman:virtual public CEmployee{
public:
    CSalesman(int nNum)
        :CEmployee(nNum),m_dSalesMoney(0.0),m_dProportion(0.0){}
    void SetSalesMoney(double dSalesMoney)
    {
        m_dSalesMoney=dSalesMoney;
    }
    void SetProportion(double dProportion)
    {
        m_dProportion=dProportion;
    }

    virtual void ComputeSalary()
    {
        m_dSalary=m_dSalesMoney*m_dProportion;
    }
protected:
    double m_dSalesMoney;
    double m_dProportion;
};

//����
class CManager:virtual public CEmployee{
public:
    CManager(int nNum)
        :CEmployee(nNum),m_dFixedSalary(0.0){}
   
    void SetFixedSalary(double dFixedSalary)
    {
        m_dFixedSalary=dFixedSalary;
    }
    virtual void ComputeSalary()
    {
        m_dSalary=m_dFixedSalary;
    }
protected:
    double m_dFixedSalary;
};

//���۾���
class CSalesManager:public CSalesman,public CManager{
public:
    CSalesManager(int nNum)
        :CEmployee(nNum),CSalesman(nNum),CManager(nNum){}

    void SetSalesMoney(double dSalesMoney)
    {
        CSalesman::SetSalesMoney(dSalesMoney);
    }
    void SetProportion(double dProportion)
    {
        CSalesman::SetProportion(dProportion);
    }
    void SetFixedSalary(double dFixedSalary)
    {
        CManager::SetFixedSalary(dFixedSalary);
    }
    virtual void ComputeSalary()
    {
        m_dSalary=m_dSalesMoney*m_dProportion+m_dFixedSalary;
    }

};

#endif