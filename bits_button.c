#include "bits_button.h"
#include <string.h>

static bits_button_t bits_btn_entity;
static bits_btn_debug_printf_func debug_printf = NULL;
static void debug_print_binary(key_value_type_t num);

// Internal state machine states (not exposed to users)
typedef enum {
    BTN_STATE_IDLE              ,
    BTN_STATE_PRESSED           ,
    BTN_STATE_LONG_PRESS        ,
    BTN_STATE_RELEASE           ,
    BTN_STATE_RELEASE_WINDOW    ,
    BTN_STATE_FINISH
} bits_btn_state_t;

static bits_btn_event_t state_to_event(bits_btn_state_t state)
{
    switch (state) {
        case BTN_STATE_PRESSED:    return BTN_EVENT_PRESSED;
        case BTN_STATE_LONG_PRESS: return BTN_EVENT_LONG_PRESS;
        case BTN_STATE_RELEASE:    return BTN_EVENT_RELEASE;
        case BTN_STATE_FINISH:     return BTN_EVENT_FINISH;
        default:                   return (bits_btn_event_t)0;
    }
}

// ============================================================================
// Buffer Implementation Selection
// ============================================================================

#ifdef BITS_BTN_DISABLE_BUFFER

// Disabled buffer mode - no buffer operations
static const bits_btn_buffer_ops_t *bits_btn_buffer_ops = NULL;

#elif defined(BITS_BTN_USE_USER_BUFFER)

// User buffer mode - buffer operations set by user
static const bits_btn_buffer_ops_t *bits_btn_buffer_ops = NULL;

void bits_button_set_buffer_ops(const bits_btn_buffer_ops_t *user_buffer_ops)
{
    if (user_buffer_ops != NULL)
    {
        bits_btn_buffer_ops = user_buffer_ops;
    }
}

#else
// Default C11 atomic buffer implementation
#include <stdatomic.h>

#ifndef BITS_BTN_BUFFER_SIZE
#define BITS_BTN_BUFFER_SIZE        10
#endif

typedef struct
{
    bits_btn_result_t buffer[BITS_BTN_BUFFER_SIZE];
    atomic_size_t read_idx;   // Atomic read index
    atomic_size_t write_idx;  // Atomic write index
} bits_btn_ring_buffer_t;

static atomic_size_t overwrite_count = 0;
static bits_btn_ring_buffer_t ring_buffer;


/**
  * @brief  Initialize the ring buffer for button results.
  * @retval None
  */
static void bits_btn_init_buffer_c11(void)
{
    bits_btn_ring_buffer_t *buf = &ring_buffer;

    atomic_init(&buf->read_idx, 0);
    atomic_init(&buf->write_idx, 0);
    atomic_init(&overwrite_count, 0);
}

static uint8_t bits_btn_is_buffer_empty_c11(void)
{
    bits_btn_ring_buffer_t *buf = &ring_buffer;

    size_t current_read = atomic_load_explicit(&buf->read_idx, memory_order_relaxed);
    size_t current_write = atomic_load_explicit(&buf->write_idx, memory_order_relaxed);
    return current_read == current_write;
}

static uint8_t bits_btn_is_buffer_full_c11(void)
{
    bits_btn_ring_buffer_t *buf = &ring_buffer;

    size_t current_write = atomic_load_explicit(&buf->write_idx, memory_order_relaxed);
    size_t current_read = atomic_load_explicit(&buf->read_idx, memory_order_relaxed);
    size_t next_write = (current_write + 1) % BITS_BTN_BUFFER_SIZE;
    return next_write == current_read;
}

static size_t get_bits_btn_buffer_used_count_c11(void)
{
    bits_btn_ring_buffer_t *buf = &ring_buffer;
    size_t current_write = atomic_load_explicit(&buf->write_idx, memory_order_relaxed);
    size_t current_read = atomic_load_explicit(&buf->read_idx, memory_order_relaxed);

    if (current_write >= current_read)
    {
        return current_write - current_read;
    }
    else
    {
        return BITS_BTN_BUFFER_SIZE - current_read + current_write;
    }
}

static size_t get_bits_btn_buffer_capacity_c11(void)
{
    // A circular buffer needs to reserve 1 empty slot to distinguish between full and empty states,
    // so the actual available capacity is SIZE - 1.
    return BITS_BTN_BUFFER_SIZE - 1;
}

/**
  * @brief  Clear the ring buffer. Note that additional synchronization is required in a multi-threaded environment.
  * @retval None
  */
static void bits_btn_clear_buffer_c11(void)
{
    bits_btn_ring_buffer_t *buf = &ring_buffer;

    atomic_store_explicit(&buf->write_idx, 0, memory_order_release);
    atomic_store_explicit(&buf->read_idx, 0, memory_order_release);
}

static size_t get_bits_btn_buffer_overwrite_count_c11(void)
{
    return atomic_load_explicit(&overwrite_count, memory_order_relaxed);
}
#if 0
/**
  * @brief  Write a button result to the ring buffer.
  * @param  result: Pointer to the button result to be written.
  * @retval true if written successfully, false if the buffer is full.
  */
static uint8_t bits_btn_write_buffer_c11(bits_btn_result_t *result)
{
    bits_btn_ring_buffer_t *buf = &ring_buffer;

    if(result == NULL)
        return false;

    size_t current_write = atomic_load_explicit(&buf->write_idx, memory_order_relaxed);
    size_t current_read = atomic_load_explicit(&buf->read_idx, memory_order_consume);

    // Calculate the next write position (using modulo operation)
    size_t next_write = (current_write + 1) % BITS_BTN_BUFFER_SIZE;

    if (next_write == current_read) {  // Buffer is full
        atomic_fetch_add_explicit(&overwrite_count, 1, memory_order_relaxed);
        return false;
    }

    buf->buffer[current_write] = *result;

    // Update the write index (ensure data is visible to other threads)
    atomic_store_explicit(&buf->write_idx, next_write, memory_order_release);
    return true;
}
#endif

/**
  * @brief  Write a button result to the ring buffer with overwrite in a single-writer scenario.
  * @param  result: Pointer to the button result to be written.
  * @retval true if written successfully.
  */
static uint8_t bits_btn_write_buffer_overwrite_c11(bits_btn_result_t *result)
{
    bits_btn_ring_buffer_t *buf = &ring_buffer;

    if(result == NULL)
        return false;
    // Get the current write position
    size_t current_write = atomic_load_explicit(&buf->write_idx, memory_order_relaxed);
    size_t next_write = (current_write + 1) % BITS_BTN_BUFFER_SIZE;
    size_t current_read = atomic_load_explicit(&buf->read_idx, memory_order_acquire);


    // Advance the read pointer when the buffer is full
    if (next_write == current_read) {
        atomic_fetch_add_explicit(&overwrite_count, 1, memory_order_relaxed);
        size_t new_read = (current_read + 1) % BITS_BTN_BUFFER_SIZE;

        atomic_store_explicit(&buf->read_idx, new_read, memory_order_release);
        current_read = new_read;
    }

    // Write data (space is guaranteed to be safe at this point)
    buf->buffer[current_write] = *result;

    // Update the write pointer (ensure data is visible before index update)
    atomic_store_explicit(&buf->write_idx, next_write, memory_order_release);
    return true;
}

/**
  * @brief  Read a button result from the ring buffer.
  * @param  result: Pointer to store the read button result.
  * @retval true if read successfully, false if the buffer is empty.
  */
static uint8_t bits_btn_read_buffer_c11(bits_btn_result_t *result)
{
    bits_btn_ring_buffer_t *buf = &ring_buffer;

    size_t current_write = atomic_load_explicit(&buf->write_idx, memory_order_acquire);
    size_t current_read = atomic_load_explicit(&buf->read_idx, memory_order_relaxed);

    if (current_read == current_write) {  // Buffer is empty
        return false;
    }

    *result = buf->buffer[current_read];

    // Update the read index (using modulo operation)
    atomic_store_explicit(&buf->read_idx, (current_read + 1) % BITS_BTN_BUFFER_SIZE, memory_order_release);

    return true;
}

/**
 * @brief  Peek a button result from the ring buffer without removing it.
 * @param  result: Pointer to store the peeked button result.
 * @retval true if peek successfully, false if the buffer is empty.
 */
static uint8_t bits_btn_peek_buffer_c11(bits_btn_result_t *result)
{
    bits_btn_ring_buffer_t *buf = &ring_buffer;

    size_t current_write = atomic_load_explicit(&buf->write_idx, memory_order_acquire);
    size_t current_read = atomic_load_explicit(&buf->read_idx, memory_order_relaxed);

    if (current_read == current_write) {  // Buffer is empty
        return false;
    }

    *result = buf->buffer[current_read];  // Read without moving the read pointer

    return true;
}

const bits_btn_buffer_ops_t c11_buffer_ops = {
    .init = bits_btn_init_buffer_c11,
    .write = bits_btn_write_buffer_overwrite_c11,
    .read = bits_btn_read_buffer_c11,
    .is_empty = bits_btn_is_buffer_empty_c11,
    .is_full = bits_btn_is_buffer_full_c11,
    .get_buffer_used_count = get_bits_btn_buffer_used_count_c11,
    .clear = bits_btn_clear_buffer_c11,
    .get_buffer_overwrite_count = get_bits_btn_buffer_overwrite_count_c11,
    .get_buffer_capacity = get_bits_btn_buffer_capacity_c11,
    .peek = bits_btn_peek_buffer_c11,  // Add peek function to the structure
};

static const bits_btn_buffer_ops_t *bits_btn_buffer_ops = &c11_buffer_ops;

#endif

#ifndef BITS_BTN_DISABLE_BUFFER
static bits_btn_result_user_filter_callback bits_btn_result_user_filter_cb = NULL;

void bits_btn_register_result_filter_callback(bits_btn_result_user_filter_callback cb)
{
    if(cb != NULL)
    {
        bits_btn_result_user_filter_cb = cb;
    }
}
#endif

/**
  * @brief  Find the index of a button object by its key ID within the button array.
  *         This is a helper function used internally to map a key ID to its corresponding
  *         index in the button array initialized via `bits_button_init()`.
  *
  * @param  key_id: The unique identifier of the button to locate.
  *
  * @retval Index of the button in the array if found (0 to N-1), or -1 if the key ID is invalid.
  */
static int _get_btn_index_by_key_id(uint16_t key_id)
{
    bits_button_t *button = &bits_btn_entity;
    for (size_t i = 0; i < button->btns_cnt; i++)
    {
        if (button->btns[i].key_id == key_id)
        {
            return i;
        }
    }

    return -1;
}

static uint32_t get_button_tick(void)
{
    return bits_btn_entity.btn_tick;
}

uint8_t bits_btn_is_buffer_empty(void)
{
    if (bits_btn_buffer_ops && bits_btn_buffer_ops->is_empty)
    {
        return bits_btn_buffer_ops->is_empty();
    }
    return true;
}

uint8_t bits_btn_is_buffer_full(void)
{
    if (bits_btn_buffer_ops && bits_btn_buffer_ops->is_full)
    {
        return bits_btn_buffer_ops->is_full();
    }
    return true;
}

size_t get_bits_btn_buffer_used_count(void)
{
    if (bits_btn_buffer_ops && bits_btn_buffer_ops->get_buffer_used_count)
    {
        return bits_btn_buffer_ops->get_buffer_used_count();
    }
    return 0;
}

/**
  * @brief  Clear the ring buffer. Note that additional synchronization is required in a multi-threaded environment.
  * @retval None
  */
void bits_btn_clear_buffer(void)
{
    if (bits_btn_buffer_ops && bits_btn_buffer_ops->clear)
    {
        bits_btn_buffer_ops->clear();
    }
}

size_t get_bits_btn_buffer_overwrite_count(void)
{
    if (bits_btn_buffer_ops && bits_btn_buffer_ops->get_buffer_overwrite_count)
    {
        return bits_btn_buffer_ops->get_buffer_overwrite_count();
    }
    return 0;
}

size_t get_bits_btn_buffer_capacity(void)
{
    if (bits_btn_buffer_ops && bits_btn_buffer_ops->get_buffer_capacity)
    {
        return bits_btn_buffer_ops->get_buffer_capacity();
    }
    return 0;
}

/**
  * @brief  Sort combo buttons during initialization (descending by key count)
  * @param  button: Pointer to button object
  * @retval None
  */
 static void sort_combo_buttons_in_init(bits_button_t *button)
 {
    const uint16_t cnt = button->btns_combo_cnt;  // Get number of combo buttons

    // Skip sorting if no combo buttons or only one
    if (cnt <= 1)
    {
        if (debug_printf && cnt == 0) debug_printf("No combo buttons\n");
        return;
    }

    // Initialize index array (0,1,2,...)
    for (uint16_t i = 0; i < cnt; i++)
        button->combo_sorted_indices[i] = i;

    // Insertion sort (descending by key count)
    for (uint16_t i = 1; i < cnt; i++)
    {
        const uint16_t temp_idx = button->combo_sorted_indices[i];  // Current element to insert
        const uint8_t temp_keys = button->btns_combo[temp_idx].key_count;  // Key count of current element
        int16_t j = i - 1;  // Start from end of sorted portion

        // Find insertion position: higher key count has higher priority
        while (j >= 0 &&
               button->btns_combo[button->combo_sorted_indices[j]].key_count < temp_keys)
        {
            // Shift lower priority elements backward
            button->combo_sorted_indices[j + 1] = button->combo_sorted_indices[j];
            j--;
        }
        // Insert current element at correct position
        button->combo_sorted_indices[j + 1] = temp_idx;
    }

#if 0
    // Debug output of sorting results
    if (debug_printf)
    {
        debug_printf("Sorted combo indices (%d):\n", cnt);
        for (uint16_t i = 0; i < cnt; i++)
        {
            const button_obj_combo_t* c = &button->btns_combo[button->combo_sorted_indices[i]];
            debug_printf("  %d: ID=%d, Keys=", i, c->btn.key_id);
            for (uint8_t j = 0; j < c->key_count; j++)
                debug_printf("%d ", c->key_single_ids[j]);
            debug_printf("\n");
        }
    }
#endif
}

int32_t bits_button_init(const bits_btn_config_t *config)
{
    bits_button_t *button = &bits_btn_entity;

    if (config == NULL)
    {
        return BITS_BTN_ERR_INVALID_PARAM;
    }

    debug_printf = config->bits_btn_debug_printf;

    if ((config->btns == NULL)
    || (config->btns_cnt == 0)
    || (config->read_button_level_func == NULL)
    || (config->btns_combo_cnt > 0 && config->btns_combo == NULL))
    {
        if(debug_printf)
            debug_printf("Invalid init parameters !\n");
        return BITS_BTN_ERR_INVALID_PARAM;
    }

    if (config->btns_cnt > BITS_BTN_MAX_BUTTONS)
    {
        if (debug_printf)
            debug_printf("Error: Too many buttons (%d > max %d)\n", config->btns_cnt, (int)BITS_BTN_MAX_BUTTONS);
        return BITS_BTN_ERR_TOO_MANY_BUTTONS;
    }

    memset(button, 0, sizeof(bits_button_t));

    button->btns = config->btns;
    button->btns_cnt = config->btns_cnt;
    button->btns_combo = config->btns_combo;
    button->btns_combo_cnt = config->btns_combo_cnt;
    button->_read_button_level = config->read_button_level_func;
    button->bits_btn_result_cb = config->bits_btn_result_cb;

    if (config->btns_combo_cnt > BITS_BTN_MAX_COMBO_BUTTONS)
    {
        if (debug_printf)
        {
            debug_printf("Error: Too many combo buttons (%d > max %d)\n",
                         config->btns_combo_cnt, BITS_BTN_MAX_COMBO_BUTTONS);
        }
        return BITS_BTN_ERR_TOO_MANY_COMBOS;
    }

    // Check single button param pointers
    for (uint16_t i = 0; i < config->btns_cnt; i++)
    {
        if (config->btns[i].param == NULL)
        {
            if (debug_printf)
                debug_printf("Error: Button[%d] param is NULL\n", i);
            return BITS_BTN_ERR_BTN_PARAM_NULL;
        }
    }

    // Check combo button param pointers
    for (uint16_t i = 0; i < config->btns_combo_cnt; i++)
    {
        if (config->btns_combo[i].btn.param == NULL)
        {
            if (debug_printf)
                debug_printf("Error: Combo button[%d] param is NULL\n", i);
            return BITS_BTN_ERR_COMBO_PARAM_NULL;
        }
    }

    // Check combo button keys configuration validity
    for (uint16_t i = 0; i < config->btns_combo_cnt; i++)
    {
        if (config->btns_combo[i].key_single_ids == NULL ||
            config->btns_combo[i].key_count == 0)
        {
            if (debug_printf)
                debug_printf("Error: Combo button[%d] has invalid keys config (key_single_ids=%p, key_count=%d)\n",
                             i,
                             (void*)config->btns_combo[i].key_single_ids,
                             config->btns_combo[i].key_count);
            return BITS_BTN_ERR_COMBO_KEYS_INVALID;
        }
    }

    for(uint16_t i = 0; i < config->btns_combo_cnt; i++)
    {
        button_obj_combo_t *combo = &button->btns_combo[i];
        combo->combo_mask = 0;

        for(uint16_t j = 0; j < combo->key_count; j++)
        {
            int idx = _get_btn_index_by_key_id(combo->key_single_ids[j]);
            if (idx == -1)
            {
                if(debug_printf)
                    debug_printf("Error, get_btn_index failed! \n");
                return BITS_BTN_ERR_INVALID_COMBO_ID;
            }
            combo->combo_mask |= ((button_mask_type_t)1UL << idx);
        }
    }

    // Sort the combination buttons during initialization.
    sort_combo_buttons_in_init(button);

#ifdef BITS_BTN_USE_USER_BUFFER
    if (bits_btn_buffer_ops == NULL)
    {
        if (debug_printf) debug_printf("Error: External buffer mode requires setting buffer ops!\n");
        return BITS_BTN_ERR_BUFFER_OPS_NULL;
    }
#endif

    if (bits_btn_buffer_ops && bits_btn_buffer_ops->init)
    {
        bits_btn_buffer_ops->init();
    }

    return BITS_BTN_OK;
}

/**
  * @brief  Get the button key result from the buffer.
  * @param  result: Pointer to store the button key result
  * @retval true if read successfully, false if the buffer is empty.
  */
uint8_t bits_button_get_key_result(bits_btn_result_t *result)
{
    if (bits_btn_buffer_ops && bits_btn_buffer_ops->read)
    {
        return bits_btn_buffer_ops->read(result);
    }
    return false;
}

/**
 * @brief  Peek the button key result from the buffer without removing it.
 * @param  result: Pointer to store the button key result
 * @retval true(1) if peek successfully, false if the buffer is empty.
 */
uint8_t bits_button_peek_key_result(bits_btn_result_t *result)
{
    if (bits_btn_buffer_ops && bits_btn_buffer_ops->peek)
    {
        return bits_btn_buffer_ops->peek(result);
    }
    return false;
}

/**
  * @brief  Reset all button states to idle.
  *         This function should be called when resuming from low power mode
  *         to clear any residual button states from before the pause.
  * @retval None
  */
void bits_button_reset_states(void)
{
    bits_button_t *button = &bits_btn_entity;

    if (debug_printf)
        debug_printf("Resetting all button states\n");

    // Reset all individual buttons
    for (size_t i = 0; i < button->btns_cnt; i++)
    {
        button->btns[i].current_state = BTN_STATE_IDLE;
        button->btns[i].last_state = BTN_STATE_IDLE;
        button->btns[i].state_bits = 0;
        button->btns[i].state_entry_time = 0;
        button->btns[i].long_press_period_trigger_cnt = 0;
    }

    // Reset all combo buttons
    if (button->btns_combo != NULL && button->btns_combo_cnt > 0)
    {
        for (size_t i = 0; i < button->btns_combo_cnt; i++)
        {
            button_obj_combo_t *combo = &button->btns_combo[i];
            combo->btn.current_state = BTN_STATE_IDLE;
            combo->btn.last_state = BTN_STATE_IDLE;
            combo->btn.state_bits = 0;
            combo->btn.state_entry_time = 0;
            combo->btn.long_press_period_trigger_cnt = 0;
        }
    }

    // Reset global button state and force mask synchronization
    // This prevents spurious release events after reset
    button_mask_type_t current_physical_mask = 0;
    for(size_t i = 0; i < button->btns_cnt; i++)
    {
        uint8_t read_gpio_level = button->_read_button_level(&button->btns[i]);
        if (read_gpio_level == button->btns[i].active_level)
        {
            current_physical_mask |= ((button_mask_type_t)1UL << i);
        }
    }

    button->current_mask = current_physical_mask;
    button->last_mask = current_physical_mask;
    button->state_entry_time = get_button_tick();

    // Clear the event buffer
    bits_btn_clear_buffer();
}

/**
  * @brief  Add a bit to the end of the number
  * @param  state_bits: src number point.
  * @param  bit: tartget bit
  * @retval none.
  */
static void __append_bit(state_bits_type_t* state_bits, uint8_t bit)
{
    *state_bits = (*state_bits << 1) | bit;
}

/**
  * @brief  check number if match the target bits
  * @param  state_bits: src number point.
  * @param  target: tartget bits
  * @param  target_bits_number: target bits number
  * @retval 1 if it matches, 0 otherwise.
  */
static uint8_t __check_if_the_bits_match(const key_value_type_t *state_bits, key_value_type_t target, uint8_t target_bits_number)
{
    key_value_type_t mask = (1 << target_bits_number) - 1;

    return (((*state_bits) & mask) == target? 1 : 0);
}

#if 0
uint8_t check_is_repeat_click_mode(struct button_obj_t* button)
{
    key_value_type_t kv_input = button->key_value;

    /* Check if the two least significant bits form 0b10 */
    if((kv_input & 0b11) != 0b10)
        return 0;

    /* Calculate the XOR result */
    key_value_type_t xor_result = kv_input ^ (kv_input >> 1);

    /* Check if xor_result + 1 is a power of 2
       This means all bits except the least significant one are 1 */
    return (xor_result != 0) && (((xor_result + 1) & (xor_result - 1)) == 0);
}
#endif

/**
  * @brief  Report a button event.
  * @param  button: Pointer to the button object.
  * @param  result: Pointer to the button result to be reported.
  * @retval None
  */
static void bits_btn_report_event(struct button_obj_t* button, bits_btn_result_t *result)
{
    bits_btn_result_callback btn_result_cb = bits_btn_entity.bits_btn_result_cb;

    if(result == NULL) return;

    if(debug_printf)
        debug_printf("key id[%d],event:%d, long trigger_cnt:%d, key_value:", result->key_id, result->event ,result->long_press_period_trigger_cnt);
    debug_print_binary(result->key_value);

#ifndef BITS_BTN_DISABLE_BUFFER
    uint8_t is_user_result_filter_exist = (bits_btn_result_user_filter_cb != NULL);
    uint8_t default_result_filter_triger = (result->event == BTN_EVENT_LONG_PRESS) || (result->event == BTN_EVENT_FINISH);

    if (bits_btn_buffer_ops && bits_btn_buffer_ops->write)
    {
        uint8_t should_write_to_buffer = 0;

        if (is_user_result_filter_exist)
        {
            should_write_to_buffer = bits_btn_result_user_filter_cb(*result);
        }
        else
        {
            should_write_to_buffer = default_result_filter_triger;
        }

        if (should_write_to_buffer)
        {
            bits_btn_buffer_ops->write(result);
        }
    }
#endif

    if(btn_result_cb)
        btn_result_cb(button, *result);

}

/**
  * @brief  Update the button state machine.
  * @param  button: Pointer to the button object.
  * @param  btn_pressed: Flag indicating whether the button is pressed.
  * @retval None
  */
static void update_button_state_machine(struct button_obj_t* button, uint8_t btn_pressed)
{
    uint32_t current_time = get_button_tick();
    uint32_t time_diff = current_time - button->state_entry_time;
    bits_btn_result_t result = {0};
    result.key_id = button->key_id;

    if(button->param == NULL)
        return;

    switch (button->current_state)
    {
        case BTN_STATE_IDLE:
            if (btn_pressed)
            {
                __append_bit(&button->state_bits, 1);

                button->current_state = BTN_STATE_PRESSED;
                button->state_entry_time = current_time;

                result.key_value = button->state_bits;
                result.event = state_to_event((bits_btn_state_t)button->current_state);
                bits_btn_report_event(button, &result);
            }
            break;
        case BTN_STATE_PRESSED:
            if (time_diff * BITS_BTN_TICKS_INTERVAL > button->param->long_press_start_time_ms)
            {
                __append_bit(&button->state_bits, 1);

                button->current_state = BTN_STATE_LONG_PRESS;
                button->state_entry_time = current_time;
                button->long_press_period_trigger_cnt = 0;

                result.key_value = button->state_bits;
                result.event = state_to_event((bits_btn_state_t)button->current_state);
                bits_btn_report_event(button, &result);
            }
            else if (btn_pressed == 0)
            {
                button->current_state = BTN_STATE_RELEASE;
            }
            break;
        case BTN_STATE_LONG_PRESS:
            if (btn_pressed == 0)
            {
                button->long_press_period_trigger_cnt = 0;
                button->current_state = BTN_STATE_RELEASE;
            }
            else if(time_diff * BITS_BTN_TICKS_INTERVAL > button->param->long_press_period_triger_ms)
            {
                button->state_entry_time = current_time;
                button->long_press_period_trigger_cnt++;

                if(__check_if_the_bits_match(&button->state_bits, 0b011, 3))
                {
                    __append_bit(&button->state_bits, 1);
                }

                result.key_value = button->state_bits;
                result.event = state_to_event((bits_btn_state_t)button->current_state);
                result.long_press_period_trigger_cnt = button->long_press_period_trigger_cnt;
                bits_btn_report_event(button, &result);
            }
            break;
        case BTN_STATE_RELEASE:
            __append_bit(&button->state_bits, 0);

            result.key_value = button->state_bits;
            result.event = BTN_EVENT_RELEASE;
            bits_btn_report_event(button, &result);

            button->current_state = BTN_STATE_RELEASE_WINDOW;
            button->state_entry_time = current_time;

            break;
        case BTN_STATE_RELEASE_WINDOW:
            if (btn_pressed)
            {
                button->current_state = BTN_STATE_IDLE;
                button->state_entry_time = current_time;
            }
            else if (time_diff * BITS_BTN_TICKS_INTERVAL > button->param->time_window_time_ms)
            {
                // Time window timeout, trigger event and return to idle
                button->current_state = BTN_STATE_FINISH;
            }
            break;
        case BTN_STATE_FINISH:

            result.key_value = button->state_bits;
            result.event = BTN_EVENT_FINISH;
            bits_btn_report_event(button, &result);

            button->state_bits = 0;
            button->current_state = BTN_STATE_IDLE;
            break;
        default:
            break;

    }

    if(button->last_state != button->current_state)
    {
#if 0
        if(debug_printf)
            debug_printf("id[%d]:cur status:%d,last:%d\n", button->key_id, button->current_state, button->last_state);
#endif
        button->last_state = button->current_state;
    }
}

/**
  * @brief  Handle the button state based on the current mask and button mask.
  * @param  button: Pointer to the button object.
  * @param  current_mask: The current button mask.
  * @param  btn_mask: The button mask of the specific button.
  * @retval None
  */
static void handle_button_state(struct button_obj_t* button, button_mask_type_t current_mask, button_mask_type_t btn_mask)
{
    uint8_t pressed = (current_mask & btn_mask) == btn_mask? 1 : 0;
    update_button_state_machine(button, pressed);
}

/**
  * @brief  Dispatch and process combo buttons and generate a suppression mask.
  * @param  button: Pointer to the bits button object.
  * @param  suppression_mask: Pointer to store the suppression mask.
  * @retval None
  */
static void dispatch_combo_buttons(bits_button_t *button, button_mask_type_t *suppression_mask)
{
    if(button->btns_combo_cnt == 0) return;

    button_mask_type_t activated_mask = 0;

    for (uint16_t i = 0; i < button->btns_combo_cnt; i++)
    {
        uint16_t combo_index = button->combo_sorted_indices[i];
        button_obj_combo_t* combo = &button->btns_combo[combo_index];
        button_mask_type_t combo_mask = combo->combo_mask;

        // Check if the current combo button is covered by a more specific combo button
        if (activated_mask & combo_mask)
        {
            // Already covered, skip processing
            continue;
        }

        // Handle state transitions for this combo button
        handle_button_state(&combo->btn, button->current_mask, combo_mask);

        if ((button->current_mask & combo_mask) == combo_mask || combo->btn.state_bits)
        {
            // Mark the current combo button as activated
            activated_mask |= combo_mask;

            if (combo->suppress)
            {
                *suppression_mask |= combo_mask;
            }
        }
    }
}

/**
  * @brief  Dispatch and process unsuppressed individual buttons.
  * @param  button: Pointer to the bits button object.
  * @param  suppression_mask: The suppression mask.
  * @retval None
  */
static void dispatch_unsuppressed_buttons(bits_button_t *button, button_mask_type_t suppression_mask)
{
    // ​​Process Unsuppressed Individual Buttons
    for (size_t i = 0; i < button->btns_cnt; i++)
    {
        button_mask_type_t btn_mask = ((button_mask_type_t)1UL << i);

        // Skip individual buttons suppressed by combo buttons
        if (suppression_mask & btn_mask) {
            continue;
        }

        handle_button_state(&button->btns[i], button->current_mask, btn_mask);
    }
}

void bits_button_ticks(void)
{
    bits_button_t *button = &bits_btn_entity;
    uint32_t current_time = get_button_tick();

    button->btn_tick++;

    // Calculate button index
    button_mask_type_t new_mask = 0;
    for(size_t i = 0; i < button->btns_cnt; i++)
    {
        uint8_t read_gpio_level = button->_read_button_level(&button->btns[i]);

        if (read_gpio_level == button->btns[i].active_level)
        {
            new_mask |= ((button_mask_type_t)1UL << i);
        }
    }

    button->current_mask = new_mask;

    // State synchronization and debounce processing
    if(button->last_mask != new_mask)
    {
        button->state_entry_time = current_time;
        if(debug_printf)
            debug_printf("NEW MASK %d\n", new_mask);
        button->last_mask = new_mask;
    }

    uint32_t time_diff = current_time - button->state_entry_time;

    if(time_diff * BITS_BTN_TICKS_INTERVAL  < BITS_BTN_DEBOUNCE_TIME_MS)
    {
        return;
    }

    button_mask_type_t suppressed_mask = 0;

    dispatch_combo_buttons(button, &suppressed_mask);

    dispatch_unsuppressed_buttons(button, suppressed_mask);
}

/**
  * @brief  Debugging function, print the input decimal number in binary format.
  * @param  None.
  * @retval None
  */
 static void debug_print_binary(key_value_type_t num) {
    if(debug_printf == NULL)
        return;

    debug_printf("0b");
    int leading_zero = 1;

    for (int i = sizeof(key_value_type_t) * 8 - 1; i >= 0; i--) {
        if ((num >> i) & 1) {
            debug_printf("1");
            leading_zero = 0;
        } else if (!leading_zero) {
            debug_printf("0");
        }
    }

    if (leading_zero) {
        debug_printf("0");
    }

    debug_printf("\r\n");
}
