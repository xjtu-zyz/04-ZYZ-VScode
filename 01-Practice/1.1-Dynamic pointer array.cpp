#include<iostream>
using namespace std;
int main() {
	int x;
	cin >> x;
    int **p=nullptr;   //题目要求使用指向指针的指针变量！
    int sum=0;
    for(int i=0;i<x;i++)
    {
    	p[i]=new int;        //p=new int *[x];
        cin>>*p[i];
        sum+=*p[i];   
    }
	cout<<sum<<endl;
	 for (int i=0;i<x;i++)
	  {
        if(p[i])delete p[i]; 
      }
	if(p) delete[]p;
    //system("pause");
	return 0;
}
