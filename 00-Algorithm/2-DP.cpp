#include <iostream>
#include <vector>
#include <algorithm>
#include <climits>
using namespace std;

int minCuttingCost(int L, vector<int> points) {
    if (points.empty()) return 0;
    sort(points.begin(), points.end());
    vector<int> cuts = {0};
    cuts.insert(cuts.end(), points.begin(), points.end());
    cuts.push_back(L);
    int n = cuts.size();
    vector<vector<int>> dp(n, vector<int>(n, 0));
    
    for (int length = 2; length < n; ++length) {
        for (int i = 0; i < n - length; ++i) {
            int j = i + length;
            dp[i][j] = INT_MAX;
            for (int k = i + 1; k < j; ++k) {
                int current_cost = dp[i][k] + dp[k][j] + (cuts[j] - cuts[i]);
                if (current_cost < dp[i][j]) {
                    dp[i][j] = current_cost;
                }
            }
        }
    }
    return dp[0][n-1];
}

// 示例用法
int main() {
    cout << minCuttingCost(10, {2, 5}) << endl;    // 输出15
    cout << minCuttingCost(10, {4, 2, 7}) << endl; // 输出20
    return 0;
}