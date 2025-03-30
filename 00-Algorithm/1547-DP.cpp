#include <iostream>
#include <algorithm>
#include <vector>
using namespace std;

void MinCost(int L, int n, int *p) {
    // 排序切割点
    sort(p, p + n + 2);

    // 定义 DP 数组
    //先将所有的切割点排序，并在数组两端加上0和L!
    vector<vector<int> > dp(n+2);
         for(int i=0 ;i<n+2; i++) //二维vector的初始化时有要求的
        {
            dp[i].resize(n+2);
        }
    // 区间 DP
    for (int len = 2; len <= n + 1; len++) { // 区间长度
        for (int i = 0; i + len <= n + 1; i++) {
            int j = i + len;
            dp[i][j] = 1e9; // 设为极大值
            for (int k = i + 1; k < j; k++) { // 选取中间切割点
                dp[i][j] = min(dp[i][j], dp[i][k] + dp[k][j] + (p[j] - p[i]));//p[j] - p[i] 是当前区间的切割代价
            }
        }
    }

    // 输出最小代价
    cout << dp[0][n + 1] << endl;
}
int main() {
	
	// 后台自动给出测试代码放在这里，无需同学编写
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
    //调用函数输出一个切割最小的代价和，结果通过cout输出，均为int类型
	MinCost(L,n,p);
    delete[] p;

    system("pause");
	return 0;
}