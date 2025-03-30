#include <iostream>
#include <algorithm>
#include <vector>
using namespace std;

void MinCost(int L, int n, int *p) {
    // �����и��
    sort(p, p + n + 2);

    // ���� DP ����
    //�Ƚ����е��и�����򣬲����������˼���0��L!
    vector<vector<int> > dp(n+2);
         for(int i=0 ;i<n+2; i++) //��άvector�ĳ�ʼ��ʱ��Ҫ���
        {
            dp[i].resize(n+2);
        }
    // ���� DP
    for (int len = 2; len <= n + 1; len++) { // ���䳤��
        for (int i = 0; i + len <= n + 1; i++) {
            int j = i + len;
            dp[i][j] = 1e9; // ��Ϊ����ֵ
            for (int k = i + 1; k < j; k++) { // ѡȡ�м��и��
                dp[i][j] = min(dp[i][j], dp[i][k] + dp[k][j] + (p[j] - p[i]));//p[j] - p[i] �ǵ�ǰ������и����
            }
        }
    }

    // �����С����
    cout << dp[0][n + 1] << endl;
}
int main() {
	
	// ��̨�Զ��������Դ�������������ͬѧ��д
	int L, n;
	cin>>L>>n;
	int *p;
	p = new int[n+2];
	p[0] = 0;
	p[n+1] = L;
	for(int i=1;i<n+1;i++)
	{
	   cin>>p[i];
    }
    //���ú������һ���и���С�Ĵ��ۺͣ����ͨ��cout�������Ϊint����
	MinCost(L,n,p);
    delete[] p;

    system("pause");
	return 0;
}