/* test_runner.c - 测试运行器实现 */
#include "test_runner.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// ==================== 全局测试运行器 ====================
test_runner_t g_test_runner = {0};

// ==================== 测试运行器函数 ====================

bool test_runner_init(bool verbose) {
    memset(&g_test_runner, 0, sizeof(test_runner_t));
    g_test_runner.verbose_mode = verbose;
    
    printf("========================================\n");
    printf("     BitsButton 测试运行器 v2.0\n");
    printf("========================================\n");
    
    return true;
}

int test_runner_register_suite(const char* suite_name, 
                               void (*setup_func)(void), 
                               void (*teardown_func)(void)) {
    if (g_test_runner.suite_count >= MAX_TEST_SUITES) {
        printf("错误: 测试套件数量超过限制 (%d)\n", MAX_TEST_SUITES);
        return -1;
    }
    
    int index = g_test_runner.suite_count;
    test_suite_t* suite = &g_test_runner.suites[index];
    
    suite->suite_name = suite_name;
    suite->setup_func = setup_func;
    suite->teardown_func = teardown_func;
    suite->test_count = 0;
    
    // 分配测试函数数组
    suite->tests = (UnityTestFunction*)malloc(MAX_TESTS_PER_SUITE * sizeof(UnityTestFunction));
    suite->test_names = (const char**)malloc(MAX_TESTS_PER_SUITE * sizeof(const char*));
    
    if (!suite->tests || !suite->test_names) {
        printf("错误: 内存分配失败\n");
        return -1;
    }
    
    g_test_runner.suite_count++;
    
    if (g_test_runner.verbose_mode) {
        printf("注册测试套件: %s\n", suite_name);
    }
    
    return index;
}

bool test_runner_add_test(int suite_index, UnityTestFunction test_func, const char* test_name) {
    if (suite_index < 0 || suite_index >= g_test_runner.suite_count) {
        printf("错误: 无效的套件索引 %d\n", suite_index);
        return false;
    }
    
    test_suite_t* suite = &g_test_runner.suites[suite_index];
    
    if (suite->test_count >= MAX_TESTS_PER_SUITE) {
        printf("错误: 套件 '%s' 测试用例数量超过限制 (%d)\n", 
               suite->suite_name, MAX_TESTS_PER_SUITE);
        return false;
    }
    
    int test_index = suite->test_count;
    suite->tests[test_index] = test_func;
    suite->test_names[test_index] = test_name;
    suite->test_count++;
    g_test_runner.total_tests++;
    
    if (g_test_runner.verbose_mode) {
        printf("  添加测试: %s\n", test_name);
    }
    
    return true;
}

int test_runner_run_suite(int suite_index) {
    if (suite_index < 0 || suite_index >= g_test_runner.suite_count) {
        printf("错误: 无效的套件索引 %d\n", suite_index);
        return -1;
    }
    
    test_suite_t* suite = &g_test_runner.suites[suite_index];
    int suite_failures = 0;
    
    printf("\n【%s】\n", suite->suite_name);
    
    for (int i = 0; i < suite->test_count; i++) {
        printf("\n=== %s ===\n", suite->test_names[i]);
        
        // 运行设置函数
        if (suite->setup_func) {
            suite->setup_func();
        }
        
        // 运行测试
        Unity.TestFailures = 0;
        Unity.CurrentTestName = suite->test_names[i];
        Unity.CurrentTestLineNumber = 0;
        
        suite->tests[i]();
        
        // 检查测试结果
        if (Unity.TestFailures == 0) {
            printf("%s:PASS\n", suite->test_names[i]);
            g_test_runner.passed_tests++;
        } else {
            printf("%s:FAIL\n", suite->test_names[i]);
            g_test_runner.failed_tests++;
            suite_failures++;
        }
        
        // 运行清理函数
        if (suite->teardown_func) {
            suite->teardown_func();
        }
    }
    
    return suite_failures;
}

int test_runner_run_all(void) {
    printf("\n开始运行所有测试...\n");
    
    int total_failures = 0;
    
    for (int i = 0; i < g_test_runner.suite_count; i++) {
        int suite_failures = test_runner_run_suite(i);
        if (suite_failures > 0) {
            total_failures += suite_failures;
        }
    }
    
    test_runner_print_summary();
    
    return total_failures;
}

void test_runner_print_summary(void) {
    printf("\n========================================\n");
    printf("           测试完成\n");
    printf("========================================\n");
    printf("\n");
    printf("总计: %d 测试, %d 通过, %d 失败\n", 
           g_test_runner.total_tests,
           g_test_runner.passed_tests, 
           g_test_runner.failed_tests);
    
    if (g_test_runner.failed_tests == 0) {
        printf("所有测试通过! ✓\n");
    } else {
        printf("有 %d 个测试失败! ✗\n", g_test_runner.failed_tests);
    }
    
    printf("========================================\n");
}

void test_runner_cleanup(void) {
    for (int i = 0; i < g_test_runner.suite_count; i++) {
        test_suite_t* suite = &g_test_runner.suites[i];
        if (suite->tests) {
            free(suite->tests);
            suite->tests = NULL;
        }
        if (suite->test_names) {
            free(suite->test_names);
            suite->test_names = NULL;
        }
    }
    
    memset(&g_test_runner, 0, sizeof(test_runner_t));
    printf("测试运行器清理完成\n");
}