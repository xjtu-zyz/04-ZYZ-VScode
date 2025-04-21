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

    // ���鲢�ҵ�ÿ�����λ������Ȩ��
    vector<int> medians;
    vector<double> medianWeights;
    for (int i = 0; i < length; i += 5) {
        int end = min(i + 5, length);
        sort(num.begin() + index + i, num.begin() + index + end);
        int pos = i + (end - i - 1) / 2;
        medians.push_back(num[index + i + pos]);
        medianWeights.push_back(weight[index + i + pos]);
    }

    // �ݹ��ҵ���λ������λ����Ϊpivot
    int numMedians = medians.size();
    WeightMedian(numMedians, medians, medianWeights, 0);
    int pivot = medians[0];
    double pivotWeight = medianWeights[0];

    // �ҵ�pivot��ԭ�����е�λ��
    int pivotIndex = -1;
    for (int i = index; i < index + length; ++i) {
        if (num[i] == pivot) {
            pivotIndex = i;
            break;
        }
    }

    // ��pivot��������ǰ���������ʼλ��
    swap(num[index], num[pivotIndex]);
    swap(weight[index], weight[pivotIndex]);

    // ��������ΪС��pivot�ʹ���pivot�Ĳ���
    int storeIndex = index + 1;
    for (int i = index + 1; i < index + length; ++i) {
        if (num[i] < pivot) {
            swap(num[i], num[storeIndex]);
            swap(weight[i], weight[storeIndex]);
            ++storeIndex;
        }
    }

    // ������ߵ�Ȩ���ܺ�
    double leftSum = 0.0;
    for (int i = index + 1; i < storeIndex; ++i) {
        leftSum += weight[i];
    }

    // �����ұߵ�Ȩ���ܺ�
    double rightSum = 0.0;
    for (int i = storeIndex; i < index + length; ++i) {
        rightSum += weight[i];
    }

    // �жϵ�ǰpivot�Ƿ�Ϊ��Ȩ��λ��
    if (leftSum <= 0.5 && rightSum <= 0.5) {
        cout << pivot << endl;
        return;
    }

    // �ݹ鴦����벿�ֻ��Ұ벿��
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