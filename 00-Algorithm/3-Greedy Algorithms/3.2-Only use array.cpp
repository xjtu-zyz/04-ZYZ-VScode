#include <iostream>
#include <algorithm>
using namespace std;

class dTree {
public:

#define MAX_N 10001
#define MAX_CHILD 1001  // ���ع���һ���ڵ������ӽڵ���

    int n, d;
    int* childCount;
    int** children;
    int** weights;
    bool* deleted;
    int result;

    dTree(int n, int d):n(n),d(d) {
        result = 0;

        childCount = new int[n];
        children = new int*[n];
        weights = new int*[n];
        deleted = new bool[n];

        for (int i = 0; i < n; ++i) {
            deleted[i] = false;
            childCount[i] = 0;
            children[i] = new int[MAX_CHILD];
            weights[i] = new int[MAX_CHILD];

            int k;
            cin >> k;
            if (k > MAX_CHILD) {
                cerr << "Exceeded max child limit at node " << i << endl;
                k = MAX_CHILD; // ����Խ�磨���õķ�ʽ�Ǵ�������������ʾ����
            }

            for (int j = 0; j < k; ++j) {
                int id, w;
                cin >> id >> w;
                children[i][j] = id;
                weights[i][j] = w;
            }
            childCount[i] = k;
        }
    }

    ~dTree() {
        for (int i = 0; i < n; ++i) {
            delete[] children[i];
            delete[] weights[i];
        }
        delete[] childCount;
        delete[] children;
        delete[] weights;
        delete[] deleted;
    }

    int dfs(int node) {
        if (childCount[node] == 0) return 0;

        int maxDepth = 0;
        for (int i = 0; i < childCount[node]; ++i) {
            int child = children[node][i];
            int weight = weights[node][i];
            int childDepth = dfs(child);
            if (!deleted[child]) {
                maxDepth = max(maxDepth, childDepth + weight);
            }
        }

        if (maxDepth > d) {
            deleted[node] = true;
            ++result;
            return -1; // ����
        }

        return (maxDepth == -1 ? 0 : maxDepth);
    }

    void solution() {
        dfs(0);
        cout << result << endl;
    }
};

int main() {
    int n, d;               //nΪ���������dΪ·������
    cin >> n >> d;
    dTree dt(n, d);    //�������ʼ����
    dt.solution();      //ͨ��solution����������
    system("pause"); 
    return 0;
}