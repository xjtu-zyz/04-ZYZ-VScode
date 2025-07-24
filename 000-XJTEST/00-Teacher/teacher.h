#ifndef _CTEACHER_H__
#define _CTEACHER_H__

#include <string>

class CTeacher{
public:
    CTeacher(int nHours) : m_nHours(nHours) {}
    virtual ~CTeacher() {}

    virtual int GetSalary() const = 0;

protected:
    int m_nHours; // 每月课时数
};

class CProfessor : public CTeacher{
public:
    CProfessor(int nHours) : CTeacher(nHours) {}
    virtual int GetSalary() const override
    {
        return 5000 + m_nHours * 50;
    }
};

class CAssociateProfessor : public CTeacher{
public:
    CAssociateProfessor(int nHours) : CTeacher(nHours) {}
    virtual int GetSalary() const override
    {
        return 3000 + m_nHours * 30;
    }
};

class CLecturer : public CTeacher{
public:
    CLecturer(int nHours) : CTeacher(nHours) {}
    virtual int GetSalary() const override
    {
        return 2000 + m_nHours * 20;
    }
};

#endif
