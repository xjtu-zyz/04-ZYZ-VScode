#include <iostream>
#include <vector>
#include <cstdlib>
using namespace std;

void WeightMedian(int length,vector<int>num, vector<double>weight,int index) 
{
	double w1 = 0;
	double w2 = 0;
	
	if (index == length) //���鳤��Ϊ0 
	{
		return;
	}
	
	for (int i = 0; i < length; i++)//���ڵ�Ȩ�غ�
	{
		if (num[i] > num[index])
		{
			w1+= weight[i];
		}
	}
	
	for (int i = 0; i < length; i++) //С�ڵ�Ȩ�غ�
	{
		if (num[i] < num[index]) 
		{
			w2+= weight[i];
		}
	}
	
	if (w1 <=0.5 && w2 <=0.5)//��Ȩ��λ���Ķ��� 
	{
		cout << num[index] << endl;
	}
	else 
	{
		WeightMedian(length,num, weight, index + 1);//�����ݹ�Ѱ�ҷ���������ֵ
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