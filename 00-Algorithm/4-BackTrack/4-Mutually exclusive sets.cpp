#include <iostream>
#include <vector>
#include <bitset>
#include <algorithm>
using namespace std;

const int ROWS = 1000;
const int COLS = 20;

bitset<ROWS> column_masks[COLS];
int conflict_masks[COLS] = {0};

int main() {
    // ��ȡ�������
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            int val;
            cin >> val;
            if (val) {
                column_masks[j].set(i);
            }
        }
    }

    // Ԥ�����ͻ����
    for (int c = 0; c < COLS; ++c) {
        for (int d = 0; d < COLS; ++d) {
            if (c == d) continue;
            if ((column_masks[c] & column_masks[d]).any()) {
                conflict_masks[c] |= (1 << d);
            }
        }
    }

    int max_total = 0;
    vector<int> best_A, best_B;

    // �������п��ܵ�A�ķǿ��Ӽ�
    for (int mask_A = 1; mask_A < (1 << COLS); ++mask_A) {
        int a_count = __builtin_popcount(mask_A);
        if (a_count == 0) continue;

        int mask_conflicts = 0;
        for (int c = 0; c < COLS; ++c) {
            if (mask_A & (1 << c)) {
                mask_conflicts |= conflict_masks[c];
            }
        }

        int candidate_mask = (~mask_A) & (~mask_conflicts);
        candidate_mask &= ((1 << COLS) - 1);  // ȷ��ֻ������20λ

        int b_count = __builtin_popcount(candidate_mask);
        if (b_count == 0) continue;

        int total = a_count + b_count;

        // ����A��B���м��ϣ�������
        vector<int> a_cols, b_cols;
        for (int i = 0; i < COLS; ++i) {
            if (mask_A & (1 << i)) {
                a_cols.push_back(i);
            }
            if (candidate_mask & (1 << i)) {
                b_cols.push_back(i);
            }
        }
        sort(a_cols.begin(), a_cols.end());
        sort(b_cols.begin(), b_cols.end());
        //�涨A����������B������
        if (a_cols.size() <= b_cols.size()) {
            continue;  // ����������������
        }
        
        // �Ƚ��߼�
        if (total > max_total) {
            max_total = total;
            best_A = a_cols;
            best_B = b_cols;
        } else if (total == max_total) {
            int current_diff = abs((int)best_A.size() - (int)best_B.size());
            int new_diff = abs(a_count - b_count);
            bool replace = false;

            if (new_diff < current_diff) {
                replace = true;
            } else if (new_diff == current_diff) {
                if (a_cols[0] < best_A[0]) {
                    replace = true;
                } else if (a_cols[0] == best_A[0]) {
                    bool new_cond = (a_cols.size() > b_cols.size());
                    bool old_cond = (best_A.size() > best_B.size());

                    if (new_cond && !old_cond) {
                        replace = true;
                    } else if (new_cond == old_cond) {
                        if (new_cond) { // ���߶���������
                            if (a_cols < best_A) {
                                replace = true;
                            }
                        } else { // ���߶��������������Ƚ��ֵ���
                            if (a_cols < best_A) {
                                replace = true;
                            }
                        }
                    }
                }
            }

            if (replace) {
                best_A = a_cols;
                best_B = b_cols;
            }
        }
    }

    // ������
    for (int col : best_A) {
        cout << col<<" ";
    }
    cout << "\n";
    for (int col : best_B) {
        cout << col<<" ";
    }
    cout << endl;

    return 0;
}