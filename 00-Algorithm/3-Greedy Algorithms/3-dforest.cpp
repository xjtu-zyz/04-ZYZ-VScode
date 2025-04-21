#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <vector>
#include <algorithm>
using namespace std;
// 算法要求：设计一个算法求 T 的最小顶点集合 S，使 T/S 为一个 d 森林。

class dTree {
public:
    int n, d;
    vector<vector<pair<int, int>>> tree;
    vector<bool> deleted;
    int result = 0;

    dTree(int n, int d) : n(n), d(d), tree(n), deleted(n, false) {
        for (int i = 0; i < n; ++i) {
            int k;
            cin >> k;
            for (int j = 0; j < k; ++j) {
                int child, weight;
                cin >> child >> weight;
                tree[i].push_back({child, weight});
            }
        }
    }

    int dfs(int node) {
        if (tree[node].empty()) return 0; // 叶子节点

        int maxDepth = 0;
        for (size_t i = 0; i < tree[node].size(); ++i) {
            int child = tree[node][i].first;
            int weight = tree[node][i].second;
            int childDepth = dfs(child);
            if (!deleted[child]) {
                maxDepth = max(maxDepth, childDepth + weight);
            }
        }
        

        if (maxDepth > d) {
            // 当前节点必须剪掉，防止路径继续增长
            deleted[node] = true;
            ++result;
            return -1; // 剪断，父节点不能再连接这个路径
        }

        return maxDepth == -1 ? 0 : maxDepth;
    }

    void solution() {
        dfs(0);
        cout << result << endl;
    }
};


//你的代码只需要补全上方dTree类来实现算法

//类所需要的其他变量、函数可自己定义编写

//只需要提交这几行代码，其他的都是后台系统自动完成的，类似于 LeetCode，下面为main函数的代码


int main() {
    int n, d;               //n为顶点个数，d为路径长度
    cin >> n >> d;
    dTree dt(n, d);    //构建与初始化树
    dt.solution();      //通过solution函数输出结果
    system("pause"); 
    return 0;
}