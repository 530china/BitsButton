
#include "bits_button.h"
#include "string.h"
#include <stdatomic.h>

// static struct button_obj_t* button_list_head = NULL;
#define POPCOUNT_TYPE_T button_mask_type_t

static bits_button_t bits_btn_entity;

#ifdef BITS_BTN_BUFFER_SIZE
typedef struct
{
    bits_btn_result_t buffer[BITS_BTN_BUFFER_SIZE];
    atomic_size_t read_idx;   // Atomic read index
    atomic_size_t write_idx;  // Atomic write index
} bits_btn_ring_buffer_t;

static atomic_size_t overwrite_count = 0;
static bits_btn_debug_printf_func debug_printf = NULL;
static bits_btn_ring_buffer_t ring_buffer;

static bool bits_btn_read_buffer(bits_btn_result_t *result);
#endif

static void debug_print_binary(key_value_type_t num);

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

#ifdef BITS_BTN_BUFFER_SIZE

/**
  * @brief  Initialize the ring buffer for button results.
  * @retval None
  */
static void bits_btn_init_buffer(void)
{
    bits_btn_ring_buffer_t *buf = &ring_buffer;

    atomic_init(&buf->read_idx, 0);
    atomic_init(&buf->write_idx, 0);
    atomic_init(&overwrite_count, 0);
}

 bool bits_btn_is_buffer_empty()
{
    bits_btn_ring_buffer_t *buf = &ring_buffer;

    size_t current_read = atomic_load_explicit(&buf->read_idx, memory_order_relaxed);
    size_t current_write = atomic_load_explicit(&buf->write_idx, memory_order_relaxed);
    return current_read == current_write;
}

bool bits_btn_is_buffer_full()
{
    bits_btn_ring_buffer_t *buf = &ring_buffer;

    size_t current_write = atomic_load_explicit(&buf->write_idx, memory_order_relaxed);
    size_t current_read = atomic_load_explicit(&buf->read_idx, memory_order_relaxed);
    size_t next_write = (current_write + 1) % BITS_BTN_BUFFER_SIZE;
    return next_write == current_read;
}

size_t get_bits_btn_buffer_count(void)
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

/**
  * @brief  Clear the ring buffer. Note that additional synchronization is required in a multi-threaded environment.
  * @retval None
  */
static void bits_btn_clear_buffer(void)
{
    bits_btn_ring_buffer_t *buf = &ring_buffer;

    atomic_store_explicit(&buf->read_idx, 0, memory_order_relaxed);
    atomic_store_explicit(&buf->write_idx, 0, memory_order_relaxed);
}

size_t get_bits_btn_overwrite_count(void)
{
    return atomic_load_explicit(&overwrite_count, memory_order_relaxed);
}

/**
  * @brief  Write a button result to the ring buffer.
  * @param  result: Pointer to the button result to be written.
  * @retval true if written successfully, false if the buffer is full.
  */
static bool bits_btn_write_buffer(bits_btn_result_t *result)
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

/**
  * @brief  Write a button result to the ring buffer with overwrite in a single-writer scenario.
  * @param  result: Pointer to the button result to be written.
  * @retval true if written successfully.
  */
static bool bits_btn_write_buffer_overwrite(bits_btn_result_t *result)
{
    bits_btn_ring_buffer_t *buf = &ring_buffer;

    if(result == NULL)
        return false;
    // Get the current write position
    size_t current_write = atomic_load_explicit(&buf->write_idx, memory_order_relaxed);
    size_t next_write = (current_write + 1) % BITS_BTN_BUFFER_SIZE;

    // Get the current read position (ensure the latest value is seen)
    size_t current_read = atomic_load_explicit(&buf->read_idx, memory_order_consume);

    buf->buffer[current_write] = *result;

    // Advance the read pointer when the buffer is full
    if (next_write == current_read) {
        atomic_fetch_add_explicit(&overwrite_count, 1, memory_order_relaxed);
        size_t new_read = (current_read + 1) % BITS_BTN_BUFFER_SIZE;

        atomic_store_explicit(&buf->read_idx, new_read, memory_order_release);
    }

    // Update the write pointer (ensure data is visible before index update)
    atomic_store_explicit(&buf->write_idx, next_write, memory_order_release);
    return true;
}

/**
  * @brief  Read a button result from the ring buffer.
  * @param  result: Pointer to store the read button result.
  * @retval true if read successfully, false if the buffer is empty.
  */
static bool bits_btn_read_buffer(bits_btn_result_t *result)
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
#endif

int32_t bits_button_init(button_obj_t* btns                                     , \
                         uint16_t btns_cnt                                      , \
                         button_obj_combo_t *btns_combo                         , \
                         uint16_t btns_combo_cnt                                , \
                         bits_btn_read_button_level read_button_level_func      , \
                         bits_btn_result_callback bits_btn_result_cb            , \
                         bits_btn_debug_printf_func bis_btn_debug_printf          \
                 )
{
    bits_button_t *button = &bits_btn_entity;
    debug_printf = bis_btn_debug_printf;

    if (btns == NULL|| read_button_level_func == NULL)
    {
        if(debug_printf)
            debug_printf("Error, bits button init failed !\n");
        return -2;
    }

    memset(button, 0, sizeof(bits_button_t));

    button->btns = btns;
    button->btns_cnt = btns_cnt;
    button->btns_combo = btns_combo;
    button->btns_combo_cnt = btns_combo_cnt;
    button->_read_button_level = read_button_level_func;
    button->bits_btn_result_cb = bits_btn_result_cb;

    for(uint16_t i = 0; i < btns_combo_cnt; i++)
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
                return -1; // Invalid ID
            }
            combo->combo_mask |= ((button_mask_type_t)1UL << idx);
        }
    }

#ifdef BITS_BTN_BUFFER_SIZE
    bits_btn_init_buffer();
#endif
    return 0;
}

/**
  * @brief  Get the button key result from the buffer.
  * @param  result: Pointer to store the button key result
  * @retval true if read successfully, false if the buffer is empty.
  */
bool bits_button_get_key_result(bits_btn_result_t *result)
{
    return bits_btn_read_buffer(result);
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
static uint8_t __check_if_the_bits_match(key_value_type_t *state_bits, key_value_type_t target, uint8_t target_bits_number)
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

#ifdef BITS_BTN_BUFFER_SIZE
    if(result->event != BTN_STATE_RELEASE)
        bits_btn_write_buffer_overwrite(result);
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
                result.event = button->current_state;
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
                result.event = button->current_state;
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
                result.event = button->current_state;
                result.long_press_period_trigger_cnt = button->long_press_period_trigger_cnt;
                bits_btn_report_event(button, &result);
            }
            break;
        case BTN_STATE_RELEASE:
            __append_bit(&button->state_bits, 0);

            result.key_value = button->state_bits;
            result.event = BTN_STATE_RELEASE;
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
            result.event = BTN_STATE_FINISH;
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
  * @brief  Calculate the number of set bits (1s) in an unsigned integer.
  * @param  w: The unsigned integer to be calculated.
  * @retval The number of set bits in the integer.
  * more see: http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
  */
static inline POPCOUNT_TYPE_T __popcount(POPCOUNT_TYPE_T w)
{
    w = w - ((w >> 1) & (POPCOUNT_TYPE_T) ~(POPCOUNT_TYPE_T)0 / 3);
    w = (w & (POPCOUNT_TYPE_T) ~(POPCOUNT_TYPE_T)0 / 15 * 3) + ((w >> 2) & (POPCOUNT_TYPE_T) ~(POPCOUNT_TYPE_T)0 / 15 * 3);
    w = (w + (w >> 4)) & (POPCOUNT_TYPE_T) ~(POPCOUNT_TYPE_T)0 / 255 * 15;
    return (POPCOUNT_TYPE_T)(w * ((POPCOUNT_TYPE_T) ~(POPCOUNT_TYPE_T)0 / 255)) >> (sizeof(POPCOUNT_TYPE_T) - 1) * 8;
}

/**
  * @brief  Sort the button combo array by key count in descending order.
  * @param  indices: Array to store the sorted indices.
  * @param  combos: Array of button combo objects.
  * @param  n: Number of button combo objects.
  * @retval None
  */
static void sort_combo_buttons_by_key_count(uint16_t indices[], button_obj_combo_t* combos, uint16_t n)
{
    for (uint16_t i = 1; i < n; ++i)
    {
        uint16_t temp = indices[i];
        int16_t j = i - 1;
        // Sort in descending order by key_count
        while (j >= 0 && combos[indices[j]].key_count < combos[temp].key_count)
        {
            indices[j + 1] = indices[j];
            j--;
        }
        indices[j + 1] = temp;
    }
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

    uint16_t sorted_indices[button->btns_combo_cnt];
    for (uint16_t i = 0; i < button->btns_combo_cnt; ++i)
    {
        sorted_indices[i] = i;
    }

    /* Sort in descending order by key_count */
    sort_combo_buttons_by_key_count(sorted_indices, button->btns_combo, button->btns_combo_cnt);

    for(uint16_t i = 0; i < button->btns_combo_cnt; i++)
    {
        button_obj_combo_t* combo = &button->btns_combo[sorted_indices[i]];
        button_mask_type_t combo_mask = combo->combo_mask;

        // Handle state transitions for this combo button
        handle_button_state(&combo->btn, button->current_mask, combo_mask);

        if (((button->current_mask & combo_mask) == combo_mask || combo->btn.state_bits)
        && combo->suppress)
        {
            *suppression_mask |= combo_mask;
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
    // ​​Process Unsuppressed Individual Buttons​
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
