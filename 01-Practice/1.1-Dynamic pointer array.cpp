#include<iostream>
using namespace std;
int main() {
	int x;
	cin >> x;
    int **p=new int*[x]();   //��ĿҪ��ʹ��ָ��ָ���ָ�������
    int sum=0;
    for(int i=0;i<x;i++)
    {
    	p[i]=new int; 
        cin>>*p[i];
        sum+=*p[i];   
    }
	cout<<sum<<endl;
	 for (int i=0;i<x;i++)
	  {
        delete p[i]; 
      }
	if(p) delete[]p;
    //system("pause");
	return 0;
}
