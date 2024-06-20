#pragma once

// 定义一个大的常量
#define LARGE 1000000

// 如果未定义TRUE，则定义TRUE为1
#if !defined TRUE
#define TRUE 1
#endif
// 如果未定义FALSE，则定义FALSE为0
#if !defined FALSE
#define FALSE 0
#endif

// 新建内存分配宏，如果分配失败则返回-1
#define NEW(x, t, n) if ((x = (t *)malloc(sizeof(t) * (n))) == 0) { return -1; }
// 释放内存并将其指针设置为0
#define FREE(x) if (x != 0) { free(x); x = 0; }
// 交换两个索引
#define SWAP_INDICES(a, b) { int_t _temp_index = a; a = b; b = _temp_index; }

// 下面的宏定义在0值预处理器指令控制下不被编译，提供了条件编译的示例
#if 0
// 使用assert断言
#define ASSERT(cond) assert(cond)
// 打印格式化字符串
#define PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__)
// 打印成本数组
#define PRINT_COST_ARRAY(a, n) \
    while (1) { \
        printf(#a" = ["); \
        if ((n) > 0) { \
            printf("%f", (a)[0]); \
            for (uint_t j = 1; j < n; j++) { \
                printf(", %f", (a)[j]); \
            } \
        } \
        printf("]\n"); \
        break; \
    }
// 打印索引数组
#define PRINT_INDEX_ARRAY(a, n) \
    while (1) { \
        printf(#a" = ["); \
        if ((n) > 0) { \
            printf("%d", (a)[0]); \
            for (uint_t j = 1; j < n; j++) { \
                printf(", %d", (a)[j]); \
            } \
        } \
        printf("]\n"); \
        break; \
    }
#else
// 在非0编译条件下，这些宏被禁用
#define ASSERT(cond)
#define PRINTF(fmt, ...)
#define PRINT_COST_ARRAY(a, n)
#define PRINT_INDEX_ARRAY(a, n)
#endif

// 数据类型定义
typedef signed int int_t;
typedef unsigned int uint_t;
typedef double cost_t;
typedef char boolean;
// 浮点格式定义
typedef enum fp_t { FP_1 = 1, FP_2 = 2, FP_DYNAMIC = 3 } fp_t;

/**
 * 实现Lapjv算法的内部函数。
 *
 * @param n 点的数量
 * @param cost 每个点的成本数组的指针
 * @param x 用于存储匹配结果的数组（x[i]匹配到y[i]）
 * @param y 用于存储匹配结果的数组（y[i]匹配到x[i]）
 * @return 成功返回0，失败返回-1
 */
extern int_t lapjv_internal(
	const uint_t n, cost_t *cost[],
	int_t *x, int_t *y);


