#include <vector>
#include <algorithm>
#include <iostream>

using namespace std;
/*
void WeightMedian(int length, vector<int> num, vector<double> weight, int index) {
    if (length <= 0) return;
    if (length == 1) {
        cout << num[index] << endl;
        return;
    }

    // 分组并找到每组的中位数及其权重
    vector<int> medians;
    vector<double> medianWeights;
    for (int i = 0; i < length; i += 5) {
        int end = min(i + 5, length);
        sort(num.begin() + index + i, num.begin() + index + end);
        int pos = i + (end - i - 1) / 2;
        medians.push_back(num[index + i + pos]);
        medianWeights.push_back(weight[index + i + pos]);
    }

    // 递归找到中位数的中位数作为pivot
    int numMedians = medians.size();
    WeightMedian(numMedians, medians, medianWeights, 0);
    int pivot = medians[0];
    double pivotWeight = medianWeights[0];

    // 找到pivot在原数组中的位置
    int pivotIndex = -1;
    for (int i = index; i < index + length; ++i) {
        if (num[i] == pivot) {
            pivotIndex = i;
            break;
        }
    }

    // 将pivot交换到当前子数组的起始位置
    swap(num[index], num[pivotIndex]);
    swap(weight[index], weight[pivotIndex]);

    // 划分数组为小于pivot和大于pivot的部分
    int storeIndex = index + 1;
    for (int i = index + 1; i < index + length; ++i) {
        if (num[i] < pivot) {
            swap(num[i], num[storeIndex]);
            swap(weight[i], weight[storeIndex]);
            ++storeIndex;
        }
    }

    // 计算左边的权重总和
    double leftSum = 0.0;
    for (int i = index + 1; i < storeIndex; ++i) {
        leftSum += weight[i];
    }

    // 计算右边的权重总和
    double rightSum = 0.0;
    for (int i = storeIndex; i < index + length; ++i) {
        rightSum += weight[i];
    }

    // 判断当前pivot是否为带权中位数
    if (leftSum <= 0.5 && rightSum <= 0.5) {
        cout << pivot << endl;
        return;
    }

    // 递归处理左半部分或右半部分
    if (leftSum > 0.5) {
        WeightMedian(storeIndex - index - 1, num, weight, index + 1);
    } else {
        int newLength = (index + length) - storeIndex;
        WeightMedian(newLength, num, weight, storeIndex);
    }
}
*/


void WeightMedian(int length, vector<int> num, vector<double> weight, int index) {
    vector<pair<int, double>> combined;
    for (int i = 0; i < length; ++i) {
        combined.push_back({num[i], weight[i]});
    }

    sort(combined.begin(), combined.end());

    double sum = 0;
    for (int i = 0; i < length; ++i) {
        sum += combined[i].second;
        if (sum >= 0.5) {
            cout << combined[i].first << endl;
            return;
        }
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
	return 0;
}