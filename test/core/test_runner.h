/* test_runner.h - 测试运行器 */
#ifndef TEST_RUNNER_H
#define TEST_RUNNER_H

#include "unity.h"
#include <stdbool.h>

// ==================== 测试运行器配置 ====================
#define MAX_TEST_SUITES 10
#define MAX_TESTS_PER_SUITE 20

// ==================== 测试套件结构 ====================
typedef struct {
    const char* suite_name;
    void (*setup_func)(void);
    void (*teardown_func)(void);
    UnityTestFunction* tests;
    const char** test_names;
    int test_count;
} test_suite_t;

// ==================== 测试运行器结构 ====================
typedef struct {
    test_suite_t suites[MAX_TEST_SUITES];
    int suite_count;
    int total_tests;
    int passed_tests;
    int failed_tests;
    bool verbose_mode;
} test_runner_t;

// ==================== 全局测试运行器 ====================
extern test_runner_t g_test_runner;

// ==================== 测试运行器函数 ====================

/**
 * @brief 初始化测试运行器
 * @param verbose 是否启用详细模式
 * @return true 成功，false 失败
 */
bool test_runner_init(bool verbose);

/**
 * @brief 注册测试套件
 * @param suite_name 套件名称
 * @param setup_func 设置函数
 * @param teardown_func 清理函数
 * @return 套件索引，-1表示失败
 */
int test_runner_register_suite(const char* suite_name, 
                               void (*setup_func)(void), 
                               void (*teardown_func)(void));

/**
 * @brief 向套件添加测试用例
 * @param suite_index 套件索引
 * @param test_func 测试函数
 * @param test_name 测试名称
 * @return true 成功，false 失败
 */
bool test_runner_add_test(int suite_index, UnityTestFunction test_func, const char* test_name);

/**
 * @brief 运行所有测试
 * @return 失败的测试数量
 */
int test_runner_run_all(void);

/**
 * @brief 运行指定套件
 * @param suite_index 套件索引
 * @return 失败的测试数量
 */
int test_runner_run_suite(int suite_index);

/**
 * @brief 打印测试统计信息
 */
void test_runner_print_summary(void);

/**
 * @brief 清理测试运行器
 */
void test_runner_cleanup(void);

// ==================== 便利宏定义 ====================

#define REGISTER_TEST_SUITE(name, setup, teardown) \
    test_runner_register_suite(name, setup, teardown)

#define ADD_TEST_TO_SUITE(suite_idx, test_func) \
    test_runner_add_test(suite_idx, test_func, #test_func)

#define RUN_ALL_TESTS() \
    test_runner_run_all()

#endif /* TEST_RUNNER_H */