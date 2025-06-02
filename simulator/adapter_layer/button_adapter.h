#pragma once
#include "../../bits_button.h"

#ifdef __cplusplus
extern "C" {
#endif

// button_adapter.h 顶部添加
#ifndef BUTTON_API
    #ifdef _WIN32
        #define BUTTON_API __declspec(dllexport)  // Windows 平台
    #else
        #define BUTTON_API __attribute__((visibility("default")))  // Linux/GCC
    #endif
#endif

// 导出函数声明（通过编译选项控制符号可见性）
void BUTTON_API button_ticks_wrapper(void);
void BUTTON_API set_key_state(uint8_t button_id, uint8_t state);
key_value_type_t BUTTON_API get_button_key_value_wrapper(struct button_obj_t* button);

#ifdef __cplusplus
}
#endif