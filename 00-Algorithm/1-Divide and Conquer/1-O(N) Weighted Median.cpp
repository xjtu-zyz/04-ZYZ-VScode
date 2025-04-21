#include <iostream>
#include <vector>
#include <cstdlib>
using namespace std;

void WeightMedian(int length,vector<int>num, vector<double>weight,int index) 
{
	double w1 = 0;
	double w2 = 0;
	
	if (index == length) //数组长度为0 
	{
		return;
	}
	
	for (int i = 0; i < length; i++)//大于的权重和
	{
		if (num[i] > num[index])
		{
			w1+= weight[i];
		}
	}
	
	for (int i = 0; i < length; i++) //小于的权重和
	{
		if (num[i] < num[index]) 
		{
			w2+= weight[i];
		}
	}
	
	if (w1 <=0.5 && w2 <=0.5)//带权中位数的定义 
	{
		cout << num[index] << endl;
	}
	else 
	{
		WeightMedian(length,num, weight, index + 1);//继续递归寻找符合条件的值
	}
}
int main() {
	int n;
	cin >> n;
	vector<int> num(n);
	vector<double> weight(n);
	for (int i = 0; i < n; ++i) {
		cin >> num[i];
	}
	for (int i = 0; i < n; ++i) {
		cin >> weight[i];
	}
	WeightMedian(n, num, weight, 0);
    system("pause");
	return 0;
}