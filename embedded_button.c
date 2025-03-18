
#include "embedded_button.h"

static struct button_obj_t* button_list_head = NULL;

#ifdef EB_DEBUG_PRINTF
static void debug_print_binary(uint32_t num);
#endif
/**
  * @brief  Initializes the button struct.
  * @param  button: the button strcut.
  * @param  read_button_func_ptr: read button level function ptr.
  * @param  active_level: GPIO level when button is pressed .
  * @param  button_id: the button id.
  * @param  map_ptr: key-value map
  * @param  map_size: map size
  * @retval None
  */
void button_init(struct button_obj_t* button            ,       \
                 uint8_t(*read_button_func_ptr)(uint8_t),       \
                 uint8_t active_level                   ,       \
                 uint8_t button_id                      ,       \
                 const key_value_match_map_t *kv_match_map_ptr, \
                 size_t map_size                                \
                 )
{
    memset(button, 0, sizeof(struct button_obj_t));
    button->key_value = (key_value_type_t)NONE_PRESS_KV;
    button->_read_button_func_ptr = read_button_func_ptr;
    button->read_level = button->_read_button_func_ptr(button_id);
    button->active_level = active_level;
    button->id = button_id;
    button->kv_match_map_ptr = kv_match_map_ptr;
    button->map_size = map_size;
}

/**
  * @brief  Inquire the button event happen.
  * @param  button: the button button strcut.
  * @retval button key value.
  */
key_value_type_t get_button_key_value(struct button_obj_t* button)
{
    return (key_value_type_t)(button->key_value);
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
  * @retval 1 is match, 0 is not match.
  */
static uint8_t __check_if_the_bits_match(key_value_type_t *state_bits, key_value_type_t target, uint8_t target_bits_number)
{
    key_value_type_t mask = (1 << target_bits_number) - 1;

    return (((*state_bits) & mask) == target? 1 : 0);
}

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

/**
  * @brief  Button driver core function, driven by 2 principles
  *         1. As long as the key value is not zero, the time tick++
  *         2. As long as the button status changes,
                    add a bit (1: press, 0: release) and
                    reset tick time (ensure that the tick is the time it was pressed or raised)
  * @param  handle: the button handle strcut.
  * @retval None
  */
void button_handler(struct button_obj_t* button)
{
    uint8_t read_gpio_level = button->_read_button_func_ptr(button->id);

    if((button->state_bits) > 0) button->ticks++;

    /*------------button debounce button---------------*/
    if(read_gpio_level != button->read_level) { //not equal to prev one
        //continue read DEBOUNCE_TICKS times same new level change
        if(++(button->debounce_cnt) >= DEBOUNCE_TICKS) {
            button->read_level = read_gpio_level;
            button->read_level_update = 1;
            button->debounce_cnt = 0;
        }
    } else { //leved not change ,counter reset.
        button->debounce_cnt = 0;
    }

    if(button->read_level_update) {
        if(button->read_level == button->active_level) {
            __append_bit(&button->state_bits, 1);
        }
        else {
            __append_bit(&button->state_bits, 0);
        }

        button->read_level_update = 0;
        button->ticks = 0;
    }

    if(button->ticks > SHORT_TICKS) {
        if(button->read_level != button->active_level) {
            button->event_analyze_en = 1;
        }
    }

    if(button->ticks > LONG_TICKS) {
        if((button->read_level == button->active_level) \
        && __check_if_the_bits_match(&button->state_bits, 0b01, 2)) // long press start
        {
            __append_bit(&button->state_bits, 1);
            button->event_analyze_en = 1;
        }
        else if((button->read_level == button->active_level) \
             && __check_if_the_bits_match(&button->state_bits, 0b011, 3)) // long press hold
        {
            __append_bit(&button->state_bits, 1);
            button->event_analyze_en = 1;
        }
    }

    if((button->state_bits) && (button->event_analyze_en)) {
        // button event processing
        button->key_value = button->state_bits;
#ifdef EB_DEBUG_PRINTF
        debug_print_binary(button->key_value);
#endif

        for(size_t i = 0; i < button->map_size; i++) {
            if(button->kv_match_map_ptr[i].kv_func_cb == NULL)
                continue;

            key_value_type_t operand_origin = button->kv_match_map_ptr[i].operand;
            key_value_type_t operand_result = button->kv_match_map_ptr[i].operand;
            kv_match_operator_type_t operator =button->kv_match_map_ptr[i].operator;
            key_value_type_t tar_result = button->kv_match_map_ptr[i].tar_result;

            if(operator == KV_MATCH_OPERATOR_NULL)
                operand_result = button->key_value;
            else if(operator & KV_MATCH_OPERATOR_BITWISE_AND)
                operand_result = (operand_origin & button->key_value);
            else if(operator & KV_MATCH_OPERATOR_BITWISE_OR)
                operand_result = (operand_origin | button->key_value);
            else if(operator & KV_MATCH_OPERATOR_BITWISE_NOT)
                operand_result = ~(button->key_value);
            else if(operator & KV_MATCH_OPERATOR_BITWISE_XOR)
                operand_result = (operand_origin ^ button->key_value);

            if(operand_result == tar_result)
            {
                button->kv_match_map_ptr[i].kv_func_cb(button);
            }
        }

        // button is released state
        if(button->read_level != button->active_level) {
            button->event_analyze_en = 0;
            button->state_bits = 0;
            button->ticks = 0;
        }

    }
     /** 2025/3/18 UESTC_Turtle_Modify **/
    //发现key_value在按键保持释放的时候不会自己清零，导致如果采用在主循环查询key_value的方式的话，会无法检测下一次相同事件的到来。
    //例如，第一次单击之后，下一次单击会检测不到
    //添加key_value清零操作，暂时还没想到更好的添加位置
    else if(button->state_bits == 0){ //如果状态位为0
          button->key_value = 0;
    }
    /***********************************/
}

/**
  * @brief  Start the button work, add the button struct into work list.
  * @param  button: target button strcut.
  * @retval 0: succeed. -1: already exist.
  */
int button_start(struct button_obj_t* button)
{
    struct button_obj_t* target = button_list_head;
    while(target) {
        if(target == button) return -1; //already exist.
        target = target->next;
    }
    button->next = button_list_head;
    button_list_head = button;

    return 0;
}

/**
  * @brief  Stop the button work, remove the button struct off work list.
  * @param  button: target button strcut.
  * @retval None
  */
void button_stop(struct button_obj_t* button)
{
    struct button_obj_t** curr;
    for(curr = &button_list_head; *curr; ) {
        struct button_obj_t* entry = *curr;
        if (entry == button) {
            *curr = entry->next;
            return;
        } else
            curr = &entry->next;
    }
}

/**
  * @brief  Background ticks, timer repeat invoking interval 5ms.
  * @param  None.
  * @retval None
  */
void button_ticks()
{
    struct button_obj_t* target;
    for(target = button_list_head; target; target = target->next) {
        button_handler(target);
    }
}

/**
  * @brief  Debugging function, print the input decimal number in binary format.
  * @param  None.
  * @retval None
  */
#ifdef EB_DEBUG_PRINTF
static void debug_print_binary(uint32_t num) {
    if (num == 0) {
        EB_DEBUG_PRINTF("0b0 \r\n");
        return;
    }

    EB_DEBUG_PRINTF("0b");
    int printed = 0;
    for (int i = 31; i >= 0; i--) {
        if ((num >> i) & 1) {
            EB_DEBUG_PRINTF("1");
            printed = 1;
        } else if (printed) {
            EB_DEBUG_PRINTF("0");
        }
    }
    if (!printed) {
        EB_DEBUG_PRINTF("0");
    }

    EB_DEBUG_PRINTF("\r\n");
}
#endif
