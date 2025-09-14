#ifndef __BITS_BUTTON_H__
#define __BITS_BUTTON_H__

#include "stdint.h"
#include "stdio.h"

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

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#define BITS_BUTTON_INIT(_key_id, _active_level, _param)                                                                                                    \
{                                                                                                                                                           \
    .key_id = _key_id, .active_level = _active_level, .param = _param,                                                                                      \
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

// Buffer operation interface for unified buffer management
typedef struct
{
    void (*init)(void);
    uint8_t (*write)(bits_btn_result_t *result);
    uint8_t (*read)(bits_btn_result_t *result);
    uint8_t (*is_empty)(void);
    uint8_t (*is_full)(void);
    size_t (*get_buffer_used_count)(void);
    void (*clear)(void);
    size_t (*get_buffer_overwrite_count)(void);
    size_t (*get_buffer_capacity)(void);
} bits_btn_buffer_ops_t;

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
  *         - -3: Too many combo buttons. The number of combo buttons exceeds the maximum allowed.
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
  * @retval true(1) if read successfully, false if the buffer is empty.
  */
 uint8_t bits_button_get_key_result(bits_btn_result_t *result);

/**
  * @brief  Reset all button states to idle.
  *         This function should be called when resuming from low power mode
  *         to clear any residual button states from before the pause.
  * @retval None
  */
void bits_button_reset_states(void);

/**
  * @brief  Get the number of buffer overwrites.
  * @retval The number of buffer overwrites.
  */
size_t get_bits_btn_buffer_overwrite_count(void);

/**
  * @brief  Get the number of button events currently stored in the buffer.
  * @retval The number of used buffer elements (pending button events).
  */
size_t get_bits_btn_buffer_used_count(void);

/**
  * @brief  Check if the ring buffer is full.
  * @retval true(1) if the buffer is full, false otherwise.
  */
uint8_t bits_btn_is_buffer_full(void);

/**
  * @brief  Check if the ring buffer is empty.
  * @retval true(1) if the buffer is empty, false otherwise.
  */
uint8_t bits_btn_is_buffer_empty(void);

/**
  * @brief  Set custom buffer operations for external buffer management.
  *         This function allows users to register their own buffer implementation
  *         by providing a set of buffer operation functions.
  * @param  user_buffer_ops: Pointer to user-defined buffer operations structure.
  *                         Pass NULL to restore default buffer operations.
  * @retval None
  * @note   This function should be called before bits_button_init().
  */
void bits_button_set_buffer_ops(const bits_btn_buffer_ops_t *user_buffer_ops);

/**
  * @brief  Get the total capacity of the button result buffer.
  *         This function returns the maximum number of button events that
  *         can be stored in the buffer.
  * @retval The total buffer capacity in number of elements. Returns 0 if buffer is disabled.
  */
size_t get_bits_btn_buffer_capacity(void);
#endif
