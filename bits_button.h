#ifndef __BITS_BUTTON_H__
#define __BITS_BUTTON_H__

#include "stdint.h"
#include "stdio.h"
#include "stdbool.h"

typedef uint32_t key_value_type_t;
typedef uint32_t state_bits_type_t;
typedef state_bits_type_t button_mask_type_t;


typedef enum {
    BTN_STATE_IDLE              ,
    BTN_STATE_PRESSED           ,
    BTN_STATE_LONG_PRESS        ,
    BTN_STATE_RELEASE           ,
    BTN_STATE_RELEASE_WINDOW    ,
    BTN_STATE_FINISH
} bits_btn_state_t;

//According to your need to modify the constants.
#ifndef BITS_BTN_TICKS_INTERVAL
#define BITS_BTN_TICKS_INTERVAL              5 //ms
#endif

#ifndef BITS_BTN_DEBOUNCE_TIME_MS
#define BITS_BTN_DEBOUNCE_TIME_MS            (40)
#endif

#define BITS_BTN_SHORT_TIME_MS               (350)
#define BITS_BTN_LONG_PRESS_START_TIME_MS    (1000)
#define BITS_BTN_LONG_PRESS_PERIOD_TRIGER_MS (1000)
#define BITS_BTN_TIME_WINDOW_TIME_MS         (300)

#ifndef BITS_BTN_BUFFER_SIZE
#define BITS_BTN_BUFFER_SIZE        10
#endif

#define BITS_BTN_NONE_PRESS_KV              0
#define BITS_BTN_SINGLE_CLICK_KV            0b010
#define BITS_BTN_DOUBLE_CLICK_KV            0b01010

#define BITS_BTN_SINGLE_CLICK_THEN_LONG_PRESS_KV     0b01011
#define BITS_BTN_DOUBLE_CLICK_THEN_LONG_PRESS_KV     0b0101011

#define BITS_BTN_LONG_PRESEE_START_KV       0b011
#define BITS_BTN_LONG_PRESEE_HOLD_KV        0b0111
#define BITS_BTN_LONG_PRESEE_HOLD_END_KV    0b01110


#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

#define BITS_BUTTON_INIT(_key_id, _active_level, _param)                                                                                                    \
    {                                                                                                                                                       \
        .key_id = _key_id, .active_level = _active_level, .param = _param,                                                                                  \
    }

#define BITS_BUTTON_COMBO_INIT(_key_id, _active_level, _param, _key_single_ids, _key_count, _single_key_suppress)                                           \
{                                                                                                                                                           \
    .key_single_ids = _key_single_ids, .key_count =  _key_count, .suppress = _single_key_suppress, .btn = BITS_BUTTON_INIT(_key_id, _active_level, _param)  \
}

typedef struct bits_btn_result
{
    uint8_t event;
    uint16_t key_id;
    uint16_t long_press_period_trigger_cnt;
    state_bits_type_t key_value;
} bits_btn_result_t;

typedef struct bits_btn_obj_param
{
    uint16_t short_press_time_ms;
    uint16_t long_press_start_time_ms;
    uint16_t long_press_period_triger_ms;
    uint16_t time_window_time_ms;
} bits_btn_obj_param_t;

typedef struct button_obj_t {
    uint8_t  active_level : 1;
    uint8_t current_state : 3;
    uint8_t last_state : 3;
    uint16_t  key_id;
    uint16_t long_press_period_trigger_cnt;
    uint32_t state_entry_time;
    state_bits_type_t state_bits;
    const bits_btn_obj_param_t *param;
} button_obj_t;

typedef uint8_t (*bits_btn_read_button_level)(struct button_obj_t *btn);
typedef void (*bits_btn_result_callback)(struct button_obj_t *btn, struct bits_btn_result button_result);
typedef int (*bits_btn_debug_printf_func)(const char*, ...);

typedef struct button_obj_combo
{
    uint8_t suppress;
    uint8_t key_count;
    uint16_t *key_single_ids;
    button_mask_type_t combo_mask;

    button_obj_t btn;
} button_obj_combo_t;

typedef struct bits_button
{
    button_obj_t *btns;
    uint16_t btns_cnt;
    button_obj_combo_t *btns_combo;
    uint16_t btns_combo_cnt;

    button_mask_type_t current_mask;
    button_mask_type_t last_mask;
    uint32_t state_entry_time;
    uint32_t btn_tick;
    bits_btn_read_button_level _read_button_level;
    bits_btn_result_callback bits_btn_result_cb;
} bits_button_t;

int32_t bits_button_init(button_obj_t* btns                                     , \
                         uint16_t btns_cnt                                      , \
                         button_obj_combo_t *btns_combo                         , \
                         uint16_t btns_combo_cnt                                , \
                         bits_btn_read_button_level read_button_level_func      , \
                         bits_btn_result_callback bits_btn_result_cb            , \
                         bits_btn_debug_printf_func bis_btn_debug_printf          \
);

void bits_button_ticks(void);
bool bits_button_get_key_result(bits_btn_result_t *result);
size_t get_bits_btn_overwrite_count(void);
size_t get_bits_btn_buffer_count(void);
bool bits_btn_is_buffer_full(void);
bool bits_btn_is_buffer_empty(void);
#endif
