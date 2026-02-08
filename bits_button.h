#ifndef __BITS_BUTTON_H__
#define __BITS_BUTTON_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>

// The maximum number of combined buttons supported
#ifndef BITS_BTN_MAX_COMBO_BUTTONS
#define BITS_BTN_MAX_COMBO_BUTTONS  8
#endif

typedef uint32_t key_value_type_t;
typedef uint32_t state_bits_type_t;
typedef state_bits_type_t button_mask_type_t;

// Maximum number of buttons based on mask type size
#define BITS_BTN_MAX_BUTTONS      (sizeof(button_mask_type_t) * 8)

/**
 * @brief BitsButton error codes for initialization and operation results.
 *        All error codes are negative values, with 0 indicating success.
 */
typedef enum {
    BITS_BTN_OK                       =  0,  // Success
    BITS_BTN_ERR_INVALID_COMBO_ID     = -1,  // Combo button references an invalid single button ID
    BITS_BTN_ERR_INVALID_PARAM        = -2,  // Invalid parameter (config/btns/read_func is NULL, etc.)
    BITS_BTN_ERR_TOO_MANY_COMBOS      = -3,  // Number of combo buttons exceeds BITS_BTN_MAX_COMBO_BUTTONS
    BITS_BTN_ERR_BUFFER_OPS_NULL      = -4,  // User buffer mode requires setting buffer ops before init
    BITS_BTN_ERR_TOO_MANY_BUTTONS     = -5,  // Number of buttons exceeds BITS_BTN_MAX_BUTTONS
    BITS_BTN_ERR_BTN_PARAM_NULL       = -6,  // Single button has NULL param pointer
    BITS_BTN_ERR_COMBO_PARAM_NULL     = -7,  // Combo button has NULL param pointer
    BITS_BTN_ERR_COMBO_KEYS_INVALID   = -8,  // Combo button keys config invalid (key_single_ids is NULL or key_count is 0)
} bits_btn_error_t;


typedef enum {
    BTN_EVENT_PRESSED    = 1,  // Button pressed (after debounce)
    BTN_EVENT_LONG_PRESS = 2,  // Long press detected or holding
    BTN_EVENT_RELEASE    = 3,  // Button released
    BTN_EVENT_FINISH     = 5,  // Button sequence completed (after time window)
} bits_btn_event_t;

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

#define BITS_BUTTON_INIT(_key_id, _active_level, _param)                                    \
{                                                                                           \
    .active_level = _active_level, .current_state = 0, .last_state = 0, .key_id = _key_id,  \
    .long_press_period_trigger_cnt = 0, .state_entry_time = 0,                              \
    .state_bits = 0, .param = _param                                                        \
}

#define BITS_BUTTON_COMBO_INIT(_key_id, _active_level, _param, _key_single_ids, _key_count, _single_key_suppress)   \
{                                                                                                                   \
    .suppress = _single_key_suppress, .key_count = _key_count, .key_single_ids = _key_single_ids, .combo_mask = 0,  \
    .btn = BITS_BUTTON_INIT(_key_id, _active_level, _param)                                                         \
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
typedef uint8_t (*bits_btn_result_user_filter_callback)(bits_btn_result_t button_result);

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
    uint8_t (*peek)(bits_btn_result_t *result);
} bits_btn_buffer_ops_t;

typedef struct
{
    button_obj_t *btns;
    uint16_t btns_cnt;
    button_obj_combo_t *btns_combo;
    uint16_t btns_combo_cnt;
    bits_btn_read_button_level read_button_level_func;
    bits_btn_result_callback bits_btn_result_cb;
    bits_btn_debug_printf_func bits_btn_debug_printf;
} bits_btn_config_t;

/**
  * @brief  Initialize the button structure and configure button detection parameters.
  *         This function sets up the button system using the provided configuration.
  *
  * @param  config: Pointer to the configuration structure containing all initialization parameters.
  *
  * @retval bits_btn_error_t Status code indicating the result of the initialization:
  *         - BITS_BTN_OK (0): Success. All parameters are valid, and the button system is initialized.
  *         - BITS_BTN_ERR_INVALID_COMBO_ID (-1): Invalid key ID in combination button configuration.
  *         - BITS_BTN_ERR_INVALID_PARAM (-2): Invalid input parameters (config/btns/read_func is NULL, etc.).
  *         - BITS_BTN_ERR_TOO_MANY_COMBOS (-3): Too many combo buttons (exceeds BITS_BTN_MAX_COMBO_BUTTONS).
  *         - BITS_BTN_ERR_BUFFER_OPS_NULL (-4): User buffer mode requires setting buffer ops before init.
  *         - BITS_BTN_ERR_TOO_MANY_BUTTONS (-5): Too many buttons (exceeds BITS_BTN_MAX_BUTTONS).
  *         - BITS_BTN_ERR_BTN_PARAM_NULL (-6): A single button has NULL param pointer.
  *         - BITS_BTN_ERR_COMBO_PARAM_NULL (-7): A combo button has NULL param pointer.
  *         - BITS_BTN_ERR_COMBO_KEYS_INVALID (-8): Combo button keys config invalid (key_single_ids is NULL or key_count is 0).
  */
int32_t bits_button_init(const bits_btn_config_t *config);

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
 * @brief  Peek the button key result from the buffer without removing it.
 * @param  result: Pointer to store the button key result
 * @retval true(1) if peek successfully, false if the buffer is empty.
 */
uint8_t bits_button_peek_key_result(bits_btn_result_t *result);

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

#ifdef BITS_BTN_USE_USER_BUFFER
/**
  * @brief  Set custom buffer operations for external buffer management.
  *         This function allows users to register their own buffer implementation
  *         by providing a set of buffer operation functions.
  * @param  user_buffer_ops: Pointer to user-defined buffer operations structure.
  *                         Pass NULL to restore default buffer operations.
  * @retval None
  * @note   This function should be called before bits_button_init().
  *         Only available when BITS_BTN_USE_USER_BUFFER is defined.
  */
void bits_button_set_buffer_ops(const bits_btn_buffer_ops_t *user_buffer_ops);
#endif

/**
  * @brief  Get the usable capacity of the button result buffer.
  *         This function returns the maximum number of button events that
  *         can be stored in the buffer.
  * @note   The underlying array size is capacity + 1 due to ring buffer
  *         implementation requirements (one slot reserved to distinguish
  *         full/empty states).
  * @retval The usable buffer capacity in number of elements. Returns 0 if buffer is disabled.
  */
size_t get_bits_btn_buffer_capacity(void);

/**
  * @brief  Register a custom filter callback for button result events.
  *         This function allows users to control which button events are written to the buffer.
  * @param  cb: Pointer to the user-defined filter callback function. The callback should
  *            return 1 to write the event to buffer, 0 to filter it out. Pass NULL to disable.
  * @retval None
  * @note   Only available in buffer mode. Default behavior writes BTN_EVENT_LONG_PRESS and BTN_EVENT_FINISH events.
  */
void bits_btn_register_result_filter_callback(bits_btn_result_user_filter_callback cb);

#ifdef __cplusplus
}
#endif

#endif
