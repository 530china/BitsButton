#ifndef __BITS_BUTTON_H__
#define __BITS_BUTTON_H__

#include "stdint.h"
#include "stdio.h"
#include "stdbool.h"

#ifndef BITS_BTN_BUFFER_SIZE
#define BITS_BTN_BUFFER_SIZE        10
#endif

#ifndef BITS_BTN_MAX_COMBO_BUTTONS
#define BITS_BTN_MAX_COMBO_BUTTONS  8 // 默认最大支持8个组合按钮
#endif

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

    uint16_t combo_sorted_indices[BITS_BTN_MAX_COMBO_BUTTONS];
} bits_button_t;

/**
  * @brief  Initialize the button structure and configure button detection parameters.
  *         This function sets up the button system, including single and combination buttons,
  *         and registers callback functions for button state changes.
  *
  * @param  btns: Pointer to an array of button objects. Each object contains button-specific
  *               parameters such as key ID, debounce time, and callback functions.
  * @param  btns_cnt: Number of button objects in the array (i.e., the number of single buttons).
  * @param  btns_combo: Pointer to an array of button combination objects. These define
  *                     multi-button presses (e.g., simultaneous key combinations).
  * @param  btns_combo_cnt: Number of button combination objects.
  * @param  read_button_level_func: Function pointer to the button level reading function.
  *                               This function should return the current state (pressed/released)
  *                               of the physical button.
  * @param  bits_btn_result_cb: Callback function pointer for button state changes.
  *                           This function is called when a button event (press, release, etc.) is detected.
  * @param  bis_btn_debug_printf: Function pointer for debug logging. Pass NULL if debug output is not needed.
  *
  * @retval Status code indicating the result of the initialization:
  *         - 0: Success. All parameters are valid, and the button system is initialized.
  *         - -1: Invalid key ID in combination button configuration. A key ID specified in a
  *               combination button configuration does not exist in the single button array.
  *         - -2: Invalid input parameters. Returned if either `btns` or `read_button_level_func` is NULL.
  */
int32_t bits_button_init(button_obj_t* btns                                     , \
                         uint16_t btns_cnt                                      , \
                         button_obj_combo_t *btns_combo                         , \
                         uint16_t btns_combo_cnt                                , \
                         bits_btn_read_button_level read_button_level_func      , \
                         bits_btn_result_callback bits_btn_result_cb            , \
                         bits_btn_debug_printf_func bis_btn_debug_printf          \
);

/**
  * @brief  Background ticks function, called repeatedly by the timer with an interval of 5ms.
  * @retval None
  */
void bits_button_ticks(void);

/**
  * @brief  Get the button key result from the buffer.
  * @param  result: Pointer to store the button key result
  * @retval true if read successfully, false if the buffer is empty.
  */
bool bits_button_get_key_result(bits_btn_result_t *result);

/**
  * @brief  Get the number of buffer overwrites.
  * @retval The number of buffer overwrites.
  */
size_t get_bits_btn_overwrite_count(void);

/**
  * @brief  Get the approximate number of elements in the ring buffer.
  * @retval The approximate number of elements in the buffer.
  */
size_t get_bits_btn_buffer_count(void);

/**
  * @brief  Check if the ring buffer is full.
  * @retval true if the buffer is full, false otherwise.
  */
bool bits_btn_is_buffer_full(void);

/**
  * @brief  Check if the ring buffer is empty.
  * @retval true if the buffer is empty, false otherwise.
  */
bool bits_btn_is_buffer_empty(void);

#endif
