#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <vector>
#include <algorithm>
using namespace std;
// �㷨Ҫ�����һ���㷨�� T ����С���㼯�� S��ʹ T/S Ϊһ�� d ɭ�֡�

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
        if (tree[node].empty()) return 0; // Ҷ�ӽڵ�

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
            // ��ǰ�ڵ�����������ֹ·����������
            deleted[node] = true;
            ++result;
            return -1; // ���ϣ����ڵ㲻�����������·��
        }

        return maxDepth == -1 ? 0 : maxDepth;
    }

    void solution() {
        dfs(0);
        cout << result << endl;
    }
};


//��Ĵ���ֻ��Ҫ��ȫ�Ϸ�dTree����ʵ���㷨

//������Ҫ�������������������Լ������д

//ֻ��Ҫ�ύ�⼸�д��룬�����Ķ��Ǻ�̨ϵͳ�Զ���ɵģ������� LeetCode������Ϊmain�����Ĵ���


int main() {
    int n, d;               //nΪ���������dΪ·������
    cin >> n >> d;
    dTree dt(n, d);    //�������ʼ����
    dt.solution();      //ͨ��solution����������
    system("pause"); 
    return 0;
}