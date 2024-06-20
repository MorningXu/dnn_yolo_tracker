#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lapjv.h"

/**
 * _ccrrt_dense函数用于执行某种密集成本计算和资源配置。
 *
 * @param n 表示成本矩阵的维度，即cost数组的行和列数。
 * @param cost 指向成本矩阵的指针数组，其中cost[i][j]表示从i到j的成本。
 * @param free_rows 用于存储自由行（未被分配）的索引的数组。
 * @param x 用于记录每个资源分配的目标列的数组。
 * @param y 用于记录当前最小成本路径来源的数组。
 * @param v 用于记录当前最小成本的数组。
 * @return 返回未分配的资源数量。
 */
int_t _ccrrt_dense(const uint_t n, cost_t *cost[],
                   int_t *free_rows, int_t *x, int_t *y, cost_t *v) {
    int_t n_free_rows;
    boolean *unique;

    // 初始化x、v和y数组。
    for (uint_t i = 0; i < n; i++) {
        x[i] = -1;
        v[i] = LARGE;
        y[i] = 0;
    }

    // 通过动态规划计算每个目标列的最小成本及其来源。
    for (uint_t i = 0; i < n; i++) {
        for (uint_t j = 0; j < n; j++) {
            const cost_t c = cost[i][j];
            if (c < v[j]) {
                v[j] = c;
                y[j] = i;
            }
            // 打印当前计算状态，用于调试。
            PRINTF("i=%d, j=%d, c[i,j]=%f, v[j]=%f y[j]=%d\n", i, j, c, v[j], y[j]);
        }
    }

    // 打印最小成本数组和来源数组，用于调试。
    PRINT_COST_ARRAY(v, n);
    PRINT_INDEX_ARRAY(y, n);

    // 创建并初始化unique数组，用于标记资源是否已被唯一分配。
    NEW(unique, boolean, n);
    memset(unique, TRUE, n); {
        int_t j = n;
        do {
            j--;
            const int_t i = y[j];
            if (x[i] < 0) {
                x[i] = j;
            } else {
                unique[i] = FALSE;
                y[j] = -1;
            }
        } while (j > 0);
    }

    // 计算并返回未分配的资源数量。
    n_free_rows = 0;
    for (uint_t i = 0; i < n; i++) {
        if (x[i] < 0) {
            free_rows[n_free_rows++] = i;
        } else if (unique[i]) {
            const int_t j = x[i];
            cost_t min = LARGE;
            // 计算并更新资源i的目标列j的最小成本。
            for (uint_t j2 = 0; j2 < n; j2++) {
                if (j2 == (uint_t) j) {
                    continue;
                }
                const cost_t c = cost[i][j2] - v[j2];
                if (c < min) {
                    min = c;
                }
            }
            // 打印更新后的最小成本，用于调试。
            PRINTF("v[%d] = %f - %f\n", j, v[j], min);
            v[j] -= min;
        }
    }

    // 释放unique数组的内存。
    FREE(unique);
    return n_free_rows;
}


/**
 * _carr_dense 函数
 *
 * 该函数在成本数组和索引数组上执行特定操作，更新成本和自由行。
 * 它遍历自由行，计算两个元素之间的最低成本，并更新相关成本和索引。
 *
 * @param n 成本数组的总元素数。
 * @param cost 指向成本数组的指针数组，每个表示每个元素的成本。
 * @param n_free_rows 初始的自由行数。
 * @param free_rows 包含自由行索引的数组。
 * @param x 用于存储索引的数组，在函数运行时更新。
 * @param y 用于存储索引的数组，在函数运行时更新。
 * @param v 用于存储成本的数组，在函数运行时更新。
 * @return 操作后新的自由行数。
 */
int_t _carr_dense(
    const uint_t n, cost_t *cost[],
    const uint_t n_free_rows,
    int_t *free_rows, int_t *x, int_t *y, cost_t *v) {
    uint_t current = 0;
    int_t new_free_rows = 0;
    uint_t rr_cnt = 0;

    // 打印数组以供调试
    PRINT_INDEX_ARRAY(x, n);
    PRINT_INDEX_ARRAY(y, n);
    PRINT_COST_ARRAY(v, n);
    PRINT_INDEX_ARRAY(free_rows, n_free_rows);

    // 主循环遍历自由行
    while (current < n_free_rows) {
        int_t i0;
        int_t j1, j2;
        cost_t v1, v2, v1_new;
        boolean v1_lowers;

        rr_cnt++;
        PRINTF("current = %d rr_cnt = %d\n", current, rr_cnt);
        const int_t free_i = free_rows[current++];
        j1 = 0;
        v1 = cost[free_i][0] - v[0];
        j2 = -1;
        v2 = LARGE;

        // 从第二个元素开始查找两个最低成本
        for (uint_t j = 1; j < n; j++) {
            PRINTF("%d = %f %d = %f\n", j1, v1, j2, v2);
            const cost_t c = cost[free_i][j] - v[j];
            if (c < v2) {
                if (c >= v1) {
                    v2 = c;
                    j2 = j;
                } else {
                    v2 = v1;
                    v1 = c;
                    j2 = j1;
                    j1 = j;
                }
            }
        }

        i0 = y[j1];
        v1_new = v[j1] - (v2 - v1);
        v1_lowers = v1_new < v[j1];
        PRINTF("%d %d 1=%d,%f 2=%d,%f v1'=%f(%d,%g) \n", free_i, i0, j1, v1, j2, v2, v1_new, v1_lowers, v[j1] - v1_new);

        // 根据计算值更新成本和自由行
        if (rr_cnt < current * n) {
            if (v1_lowers) {
                v[j1] = v1_new;
            } else if (i0 >= 0 && j2 >= 0) {
                j1 = j2;
                i0 = y[j2];
            }
            if (i0 >= 0) {
                if (v1_lowers) {
                    free_rows[--current] = i0;
                } else {
                    free_rows[new_free_rows++] = i0;
                }
            }
        } else {
            PRINTF("rr_cnt=%d >= %d (current=%d * n=%d)\n", rr_cnt, current * n, current, n);
            if (i0 >= 0) {
                free_rows[new_free_rows++] = i0;
            }
        }

        // 更新索引以备后续迭代
        x[free_i] = j1;
        y[j1] = free_i;
    }

    // 返回新的自由行数
    return new_free_rows;
}

/**
 * _find_dense函数用于在给定数组中查找并移动最小成本元素到指定位置。
 *
 * @param n 表示数组的总长度。
 * @param lo 表示查找范围的起始索引。
 * @param d 包含成本值的数组，与cols数组对应。
 * @param cols 包含待排序元素索引的数组。
 * @param y 未使用，保留参数。
 * @return 返回最小成本元素所在范围的新上限。
 */
uint_t _find_dense(const uint_t n, uint_t lo, cost_t *d, int_t *cols, int_t *y) {
    uint_t hi = lo + 1; // 初始化查找范围的上界。
    cost_t mind = d[cols[lo]]; // 初始化当前最小成本为起始位置的成本。

    // 从范围的下界之后的元素开始，查找并调整最小成本元素的位置。
    for (uint_t k = hi; k < n; k++) {
        int_t j = cols[k]; // 当前遍历到的元素的索引。
        // 如果找到更小的成本或成本相等的元素。
        if (d[j] <= mind) {
            if (d[j] < mind) {
                // 如果找到更小的成本，更新最小成本和范围上界。
                hi = lo;
                mind = d[j];
            }
            // 交换当前元素和范围上界的位置，将较小成本的元素上移。
            cols[k] = cols[hi];
            cols[hi++] = j;
        }
    }
    return hi; // 返回调整后范围的新上界。
}


/**
 * _scan_dense函数: 对密集区域进行扫描。
 *
 * 参数:
 * n - 表示列数。
 * cost - 指向一个二维数组的指针，表示成本矩阵。
 * plo - 指向一个uint_t类型的指针，表示当前扫描的低位索引。
 * phi - 指向一个uint_t类型的指针，表示当前扫描的高位索引。
 * d - 指向一个cost_t类型的数组，用于存储当前最小成本。
 * cols - 指向一个int_t类型的数组，用于存储列索引。
 * pred - 指向一个int_t类型的数组，用于存储前驱索引。
 * y - 指向一个int_t类型的数组，用于存储标签。
 * v - 指向一个cost_t类型的数组，用于存储变量值。
 *
 * 返回值:
 * 如果找到一个满足条件的索引，则返回该索引；否则返回-1。
 */
int_t _scan_dense(const uint_t n, cost_t *cost[],
                  uint_t *plo, uint_t *phi,
                  cost_t *d, int_t *cols, int_t *pred,
                  int_t *y, cost_t *v) {
    uint_t lo = *plo;
    uint_t hi = *phi;
    cost_t h, cred_ij;

    // 在lo和hi之间扫描
    while (lo != hi) {
        int_t j = cols[lo++];
        const int_t i = y[j];
        const cost_t mind = d[j];
        h = cost[i][j] - v[j] - mind;
        PRINTF("i=%d j=%d h=%f\n", i, j, h);

        // 遍历TODO中的所有列
        for (uint_t k = hi; k < n; k++) {
            j = cols[k];
            cred_ij = cost[i][j] - v[j] - h;
            if (cred_ij < d[j]) {
                d[j] = cred_ij;
                pred[j] = i;
                if (cred_ij == mind) {
                    if (y[j] < 0) {
                        return j;
                    }
                    cols[k] = cols[hi];
                    cols[hi++] = j;
                }
            }
        }
    }
    *plo = lo;
    *phi = hi;
    return -1;
}

/**
 * 密集路径查找算法。
 *
 * @param n 图中节点的数量。
 * @param cost 指向成本数组的指针数组，cost[i][j] 表示从节点 i 到节点 j 的成本。
 * @param start_i 查找的起始节点索引。
 * @param y 用于记录当前节点是否被访问过的数组，如果 y[j] < 0，则节点 j 尚未被访问。
 * @param v 用于记录从起始节点到当前节点的最短路径成本的数组。
 * @param pred 记录最短路径上每个节点的前驱节点的数组。
 * @return final_j 最终找到的目标节点索引。
 */
int_t find_path_dense(
    const uint_t n, cost_t *cost[],
    const int_t start_i,
    int_t *y, cost_t *v,
    int_t *pred) {
    uint_t lo = 0, hi = 0;
    int_t final_j = -1;
    uint_t n_ready = 0;
    int_t *cols;
    cost_t *d;

    // 初始化辅助数组 cols 和 d。
    NEW(cols, int_t, n);
    NEW(d, cost_t, n);

    // 初始化 cols, pred 和 d 数组。
    for (uint_t i = 0; i < n; i++) {
        cols[i] = i;
        pred[i] = start_i;
        d[i] = cost[start_i][i] - v[i];
    }
    // 打印初始成本数组 d。
    PRINT_COST_ARRAY(d, n);

    // 主循环，直到找到最终目标节点。
    while (final_j == -1) {
        // 如果 SCAN 列表为空，则调用 _find_dense 方法重新填充列表。
        if (lo == hi) {
            PRINTF("%d..%d -> find\n", lo, hi);
            n_ready = lo;
            hi = _find_dense(n, lo, d, cols, y);
            PRINTF("check %d..%d\n", lo, hi);
            PRINT_INDEX_ARRAY(cols, n);
            // 在 SCAN 列表中查找未访问的节点作为最终目标节点。
            for (uint_t k = lo; k < hi; k++) {
                const int_t j = cols[k];
                if (y[j] < 0) {
                    final_j = j;
                }
            }
        }
        // 如果未找到最终目标节点，则调用 _scan_dense 方法更新 SCAN 列表。
        if (final_j == -1) {
            PRINTF("%d..%d -> scan\n", lo, hi);
            final_j = _scan_dense(
                n, cost, &lo, &hi, d, cols, pred, y, v);
            PRINT_COST_ARRAY(d, n);
            PRINT_INDEX_ARRAY(cols, n);
            PRINT_INDEX_ARRAY(pred, n);
        }
    }

    // 打印最终找到的目标节点。
    PRINTF("found final_j=%d\n", final_j);
    PRINT_INDEX_ARRAY(cols, n);
    // 调整 v 数组，以反映从起始节点到所有已访问节点的最短路径成本。
    {
        const cost_t mind = d[cols[lo]];
        for (uint_t k = 0; k < n_ready; k++) {
            const int_t j = cols[k];
            v[j] += d[j] - mind;
        }
    }

    // 释放分配的内存。
    FREE(cols);
    FREE(d);

    return final_j;
}

/**
 * 函数：_ca_dense
 * ---------------
 * 对给定问题实例执行密集矩阵计算。
 *
 * 参数：
 * n - 矩阵中的总元素数量。
 * cost - 指向cost_t数组的数组，表示成本矩阵。
 * n_free_rows - 成本矩阵中自由行的数量。
 * free_rows - 存储自由行索引的整数数组。
 * x, y - 用于内部计算的数组，表示映射关系。
 * v - 用于在过程中存储计算值的数组。
 *
 * 返回值：
 * 整型值，指示操作完成状态（成功时返回0）。
 */
int_t _ca_dense(
    const uint_t n, cost_t *cost[],
    const uint_t n_free_rows,
    int_t *free_rows, int_t *x, int_t *y, cost_t *v) {
    int_t *pred;

    // 为预测数组分配内存
    NEW(pred, int_t, n);

    // 遍历每个自由行进行处理
    for (int_t *pfree_i = free_rows; pfree_i < free_rows + n_free_rows; pfree_i++) {
        int_t i = -1, j;
        uint_t k = 0;

        // 调试打印：检查当前自由行
        PRINTF("正在查看 free_i=%d\n", *pfree_i);
        // 使用密集算法寻找路径
        j = find_path_dense(n, cost, *pfree_i, y, v, pred);
        // 确保找到的路径有效
        ASSERT(j >= 0);
        ASSERT(j < n);
        // 继续直到回到原始自由行
        while (i != *pfree_i) {
            // 调试打印：增强路径
            PRINTF("增强 %d\n", j);
            // 调试打印：显示当前预测数组
            PRINT_INDEX_ARRAY(pred, n);
            // 根据找到的路径更新y数组
            i = pred[j];
            PRINTF("y[%d]=%d -> %d\n", j, y[j], i);
            y[j] = i;
            // 更新x数组以反映新的映射关系
            PRINT_INDEX_ARRAY(x, n);
            SWAP_INDICES(j, x[i]);
            // 安全检查，防止无限循环
            k++;
            if (k >= n) {
                ASSERT(FALSE);
            }
        }
    }
    // 释放预测数组分配的内存
    FREE(pred);
    // 返回成功
    return 0;
}

/**
 * 这个函数实现了Laporte-Johnson算法的内部版本，用于解决赋权匹配问题。
 *
 * @param n 表示参与匹配的节点数量。
 * @param cost 指向一个二维数组的指针，该数组包含了每对节点的匹配成本。
 * @param x 和 @param y 分别是两个整型数组，用于记录匹配结果，x[i]表示第i个节点匹配到的节点，y[i]则表示匹配到第i个节点的节点。
 * @return 返回一个整数，表示算法的执行状态：成功则返回0，否则返回一个正数，表示停止的条件。
 */
int lapjv_internal(
    const uint_t n, cost_t *cost[],
    int_t *x, int_t *y) {
    int ret;
    int_t *free_rows;
    cost_t *v;

    // 分配内存给free_rows和v，用于算法内部计算。
    NEW(free_rows, int_t, n);
    NEW(v, cost_t, n);

    // 第一次尝试使用_ccrrt_dense算法。
    ret = _ccrrt_dense(n, cost, free_rows, x, y, v);

    int i = 0;
    // 如果第一次尝试没有完全解决问题，那么尝试使用_carr_dense算法两次。
    while (ret > 0 && i < 2) {
        ret = _carr_dense(n, cost, ret, free_rows, x, y, v);
        i++;
    }

    // 如果前面的步骤仍有未匹配节点，则尝试使用_ca_dense算法。
    if (ret > 0) {
        ret = _ca_dense(n, cost, ret, free_rows, x, y, v);
    }

    // 释放分配的内存。
    FREE(v);
    FREE(free_rows);

    return ret;
}
