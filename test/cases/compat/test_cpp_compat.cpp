// C++兼容性测试文件
// 验证BitsButton库是否能在C++环境中正常编译和使用
// 覆盖关键的C++兼容性场景

#include "bits_button.h"
#include <iostream>
#include <cstdint>

// C++命名空间测试
namespace test_namespace {
    // 测试在命名空间中使用库
    static void test_in_namespace() {
        const bits_btn_obj_param_t param = {
            .short_press_time_ms = BITS_BTN_SHORT_TIME_MS,
            .long_press_start_time_ms = BITS_BTN_LONG_PRESS_START_TIME_MS,
            .long_press_period_triger_ms = BITS_BTN_LONG_PRESS_PERIOD_TRIGER_MS,
            .time_window_time_ms = BITS_BTN_TIME_WINDOW_TIME_MS
        };

        button_obj_t test_button = BITS_BUTTON_INIT(1, 1, &param);
        (void)test_button; // 避免unused变量警告
    }
}

// C++类中使用测试
class ButtonManager {
private:
    button_obj_t buttons[3];
    bits_btn_obj_param_t param;

public:
    ButtonManager() {
        // 测试在构造函数中初始化
        param = {
            .short_press_time_ms = BITS_BTN_SHORT_TIME_MS,
            .long_press_start_time_ms = BITS_BTN_LONG_PRESS_START_TIME_MS,
            .long_press_period_triger_ms = BITS_BTN_LONG_PRESS_PERIOD_TRIGER_MS,
            .time_window_time_ms = BITS_BTN_TIME_WINDOW_TIME_MS
        };

        // 测试宏初始化在类中的使用
        buttons[0] = BITS_BUTTON_INIT(1, 1, &param);
        buttons[1] = BITS_BUTTON_INIT(2, 0, &param);
        buttons[2] = BITS_BUTTON_INIT(3, 1, &param);
    }

    // 测试组合按键在类方法中的使用
    button_obj_combo_t createComboButton() {
        static uint16_t combo_keys[] = {1, 2, 3};
        return BITS_BUTTON_COMBO_INIT(
            100,            // 组合键ID
            1,              // 有效电平
            &param,         // 参数配置
            combo_keys,     // 组合按键成员
            3,              // 组合键成员数量
            1               // 抑制单键事件
        );
    }

    // 测试枚举类型在类中的使用
    bits_btn_state_t getButtonState(int index) {
        if (index >= 0 && index < 3) {
            return static_cast<bits_btn_state_t>(buttons[index].current_state);
        }
        return BTN_STATE_IDLE;
    }
};

// 模板函数测试（验证类型兼容性）
template<typename T>
static T getValue(T value) {
    return value;
}

// 函数重载测试
static void processButton(const button_obj_t& btn) {
    // 处理单按键
    (void)btn;
}

static void processButton(const button_obj_combo_t& combo_btn) {
    // 处理组合按键
    (void)combo_btn;
}

// 主要测试函数
int main() {
    std::cout << "BitsButton C++兼容性全面测试开始..." << std::endl;

    try {
        // 1. 基本结构体初始化测试
        std::cout << "1. 测试基本结构体初始化..." << std::endl;
        const bits_btn_obj_param_t test_param = {
            .short_press_time_ms = BITS_BTN_SHORT_TIME_MS,
            .long_press_start_time_ms = BITS_BTN_LONG_PRESS_START_TIME_MS,
            .long_press_period_triger_ms = BITS_BTN_LONG_PRESS_PERIOD_TRIGER_MS,
            .time_window_time_ms = BITS_BTN_TIME_WINDOW_TIME_MS
        };

        // 2. 宏初始化测试
        std::cout << "2. 测试宏初始化..." << std::endl;
        button_obj_t test_button = BITS_BUTTON_INIT(1, 1, &test_param);

        // 3. 组合按键初始化测试
        std::cout << "3. 测试组合按键初始化..." << std::endl;
        uint16_t combo_keys[] = {1, 2};
        button_obj_combo_t test_combo = BITS_BUTTON_COMBO_INIT(
            100,            // 组合键ID
            1,              // 有效电平
            &test_param,    // 参数配置
            combo_keys,     // 组合按键成员
            2,              // 组合键成员数量
            1               // 抑制单键事件
        );

        // 4. 枚举类型测试
        std::cout << "4. 测试枚举类型..." << std::endl;
        bits_btn_state_t state = BTN_STATE_IDLE;
        state = BTN_STATE_PRESSED;
        (void)state;

        // 5. 所有宏定义可用性测试
        std::cout << "5. 测试宏定义..." << std::endl;
        uint32_t time_values[] = {
            BITS_BTN_SHORT_TIME_MS,
            BITS_BTN_LONG_PRESS_START_TIME_MS,
            BITS_BTN_LONG_PRESS_PERIOD_TRIGER_MS,
            BITS_BTN_TIME_WINDOW_TIME_MS,
            BITS_BTN_DEBOUNCE_TIME_MS,
            BITS_BTN_TICKS_INTERVAL
        };

        uint32_t key_values[] = {
            BITS_BTN_NONE_PRESS_KV,
            BITS_BTN_SINGLE_CLICK_KV,
            BITS_BTN_DOUBLE_CLICK_KV,
            BITS_BTN_SINGLE_CLICK_THEN_LONG_PRESS_KV,
            BITS_BTN_DOUBLE_CLICK_THEN_LONG_PRESS_KV,
            BITS_BTN_LONG_PRESEE_START_KV,
            BITS_BTN_LONG_PRESEE_HOLD_KV,
            BITS_BTN_LONG_PRESEE_HOLD_END_KV
        };
        (void)time_values;
        (void)key_values;

        // 6. 类型定义测试
        std::cout << "6. 测试类型定义..." << std::endl;
        key_value_type_t key_val = 0;
        state_bits_type_t state_bits = 0;
        button_mask_type_t mask = 0;
        (void)key_val; (void)state_bits; (void)mask;

        // 7. 模板函数兼容性测试
        std::cout << "7. 测试模板函数兼容性..." << std::endl;
        uint16_t key_id = getValue<uint16_t>(test_button.key_id);
        uint8_t active_level = getValue<uint8_t>(test_button.active_level);
        (void)key_id; (void)active_level;

        // 8. 函数重载测试
        std::cout << "8. 测试函数重载..." << std::endl;
        processButton(test_button);
        processButton(test_combo);

        // 9. C++类使用测试
        std::cout << "9. 测试C++类使用..." << std::endl;
        ButtonManager manager;
        button_obj_combo_t combo = manager.createComboButton();
        bits_btn_state_t btn_state = manager.getButtonState(0);
        (void)combo; (void)btn_state;

        // 10. 命名空间测试
        std::cout << "10. 测试命名空间..." << std::endl;
        test_namespace::test_in_namespace();

        // 11. 结果结构体测试
        std::cout << "11. 测试结果结构体..." << std::endl;
        bits_btn_result_t result = {0};
        result.event = 1;
        result.key_id = test_button.key_id;
        result.long_press_period_trigger_cnt = 0;
        result.key_value = BITS_BTN_SINGLE_CLICK_KV;
        (void)result;

        // 12. 缓冲区操作接口测试
        std::cout << "12. 测试缓冲区操作接口..." << std::endl;
        bits_btn_buffer_ops_t buffer_ops = {0};
        (void)buffer_ops;

        // 基本验证
        if (test_button.key_id == 1 &&
            test_combo.key_count == 2 &&
            test_param.short_press_time_ms == BITS_BTN_SHORT_TIME_MS) {
            std::cout << "🎉 所有C++兼容性测试通过！" << std::endl;
            return 0;  // 成功
        } else {
            std::cout << "❌ 基本验证失败！" << std::endl;
            return 1;  // 失败
        }

    } catch (const std::exception& e) {
        std::cout << "❌ C++异常: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "❌ 未知异常" << std::endl;
        return 1;
    }
}