#include<iostream>
using namespace std;

void swap(int &a, int &b) {
    int temp = a;
    a = b;
    b = temp;
}
void Sort(int a[],int x)
{
    for(int i=0;i<x;i++)
    {
        for(int j=i+1;j<x;j++)
        {
            if(a[i]>a[j])swap(a[i],a[j]);
        }
    }

}
int main() {
	int *p=NULL;
    p=new int[3]();
    cin>>p[0]>>p[1]>>p[2];
    Sort(p,3);

	for(int i=0;i<3;i++)
    {
        cout<<p[i]<<" ";
    }
    delete[]p;
   // system("pause");
	return 0;
}
