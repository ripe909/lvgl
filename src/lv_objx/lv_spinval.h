/**
 * @file lv_spinval.h
 *
 */

#ifndef LV_SPINVAL_H
#define LV_SPINVAL_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#ifdef LV_CONF_INCLUDE_SIMPLE
#include "lv_conf.h"
#else
#include "../../../lv_conf.h"
#endif

#if LV_USE_SPINVAL != 0

/*Testing of dependencies*/
#if LV_USE_TA == 0
#error "lv_spinval: lv_ta is required. Enable it in lv_conf.h (LV_USE_TA  1) "
#endif

#include "../lv_core/lv_obj.h"
#include "../lv_objx/lv_ta.h"

/*********************
 *      DEFINES
 *********************/
#define LV_SPINVAL_MAX_DIGIT_COUNT 6

/**********************
 *      TYPEDEFS
 **********************/

/*Data of spinval*/
typedef struct
{
    lv_ta_ext_t ta; /*Ext. of ancestor*/
    /*New data for this type */
    int32_t value;
    int32_t range_max;
    int32_t range_min;
    int32_t step;
    uint16_t digit_count : 4;
    uint16_t dec_point_pos : 4; /*if 0, there is no separator and the number is an integer*/
    uint16_t digit_padding_left : 4;
    int16_t drag_value;
    uint8_t scroll_prop : 1;    /*1: Propagate the scrolling the the parent if the edge is reached*/
    uint8_t scroll_prop_ip : 1; /*1: Scroll propagation is in progress (used by the library)*/
} lv_spinval_ext_t;

/*Styles*/
enum {
    LV_SPINVAL_STYLE_BG,
    LV_SPINVAL_STYLE_SB,
    LV_SPINVAL_STYLE_CURSOR,
};
typedef uint8_t lv_spinval_style_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a spinval objects
 * @param par pointer to an object, it will be the parent of the new spinval
 * @param copy pointer to a spinval object, if not NULL then the new object will be copied from it
 * @return pointer to the created spinval
 */
lv_obj_t * lv_spinval_create(lv_obj_t * par, const lv_obj_t * copy);

/*=====================
 * Setter functions
 *====================*/

/**
 * Set a style of a spinval.
 * @param templ pointer to template object
 * @param type which style should be set
 * @param style pointer to a style
 */
static inline void lv_spinval_set_style(lv_obj_t * spinval, lv_spinval_style_t type, lv_style_t * style)
{
    lv_ta_set_style(spinval, type, style);
}

/**
 * Set spinval value
 * @param spinval pointer to spinval
 * @param i value to be set
 */
void lv_spinval_set_value(lv_obj_t * spinval, int32_t i);

void lv_spinval_set_text(lv_obj_t * spinval, int32_t i, const char * text);

/**
 * Set spinval digit format (digit count and decimal format)
 * @param spinval pointer to spinval
 * @param digit_count number of digit excluding the decimal separator and the sign
 * @param separator_position number of digit before the decimal point. If 0, decimal point is not
 * shown
 */
void lv_spinval_set_digit_format(lv_obj_t * spinval, uint8_t digit_count, uint8_t separator_position);

/**
 * Set spinval step
 * @param spinval pointer to spinval
 * @param step steps on increment/decrement
 */
void lv_spinval_set_step(lv_obj_t * spinval, uint32_t step);

/**
 * Set spinval value range
 * @param spinval pointer to spinval
 * @param range_min maximum value, inclusive
 * @param range_max minimum value, inclusive
 */
void lv_spinval_set_range(lv_obj_t * spinval, int32_t range_min, int32_t range_max);

/**
 * Set spinval left padding in digits count (added between sign and first digit)
 * @param spinval pointer to spinval
 * @param cb Callback function called on value change event
 */
void lv_spinval_set_padding_left(lv_obj_t * spinval, uint8_t padding);

/*=====================
 * Getter functions
 *====================*/

/**
 * Get style of a spinval.
 * @param templ pointer to template object
 * @param type which style should be get
 * @return style pointer to the style
 */
static inline const lv_style_t * lv_spinval_get_style(lv_obj_t * spinval, lv_spinval_style_t type)
{
    return lv_ta_get_style(spinval, type);
}

/**
 * Get the spinval numeral value (user has to convert to float according to its digit format)
 * @param spinval pointer to spinval
 * @return value integer value of the spinval
 */
int32_t lv_spinval_get_value(lv_obj_t * spinval);

/*=====================
 * Other functions
 *====================*/

/**
 * Select next lower digit for edition by dividing the step by 10
 * @param spinval pointer to spinval
 */
void lv_spinval_step_next(lv_obj_t * spinval);

/**
 * Select next higher digit for edition by multiplying the step by 10
 * @param spinval pointer to spinval
 */
void lv_spinval_step_prev(lv_obj_t * spinval);

/**
 * Increment spinval value by one step
 * @param spinval pointer to spinval
 */
void lv_spinval_increment(lv_obj_t * spinval);

/**
 * Decrement spinval value by one step
 * @param spinval pointer to spinval
 */
void lv_spinval_decrement(lv_obj_t * spinval);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_SPINVAL*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_SPINVAL_H*/
