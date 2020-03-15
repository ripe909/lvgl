/**
 * @file lv_spinval.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_spinval.h"

#if LV_USE_SPINVAL != 0
#include "../lv_core/lv_debug.h"
#include "../lv_themes/lv_theme.h"
#include "../lv_misc/lv_math.h"
#include "../lv_misc/lv_utils.h"

/*********************
 *      DEFINES
 *********************/

#define LV_SPINVAL_NOT_PRESSED INT16_MIN

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static lv_res_t lv_spinval_signal(lv_obj_t *spinval, lv_signal_t sign,
		void *param);
static lv_res_t lv_spinval_scrl_signal(lv_obj_t *roller_scrl, lv_signal_t sign,
		void *param);
static void lv_spinval_updatevalue(lv_obj_t *spinval);

static void lv_spinval_updatetext(lv_obj_t *spinval, const char * text);

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_signal_cb_t ancestor_signal;
static lv_signal_cb_t ancestor_scrl_signal;
static lv_design_cb_t ancestor_design;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Create a spinval object
 * @param par pointer to an object, it will be the parent of the new spinval
 * @param copy pointer to a spinval object, if not NULL then the new object will be copied from it
 * @return pointer to the created spinval
 */
lv_obj_t* lv_spinval_create(lv_obj_t *par, const lv_obj_t *copy) {
	LV_LOG_TRACE("spinval create started");

	/*Create the ancestor of spinval*/
	lv_obj_t *new_spinval = lv_ta_create(par, copy);
    LV_ASSERT_MEM(new_spinval);

	if (new_spinval == NULL)
		return NULL;

	/*Allocate the spinval type specific extended data*/
	lv_spinval_ext_t *ext = lv_obj_allocate_ext_attr(new_spinval,
			sizeof(lv_spinval_ext_t));
	LV_ASSERT_MEM(ext);
	if (ext == NULL)
		return NULL;
	if (ancestor_scrl_signal == NULL)
		ancestor_scrl_signal = lv_obj_get_signal_cb(
				lv_page_get_scrl(new_spinval));
	if (ancestor_signal == NULL)
		ancestor_signal = lv_obj_get_signal_cb(new_spinval);
	if (ancestor_design == NULL)
		ancestor_design = lv_obj_get_design_cb(new_spinval);

	/*Initialize the allocated 'ext'*/
	ext->value = 0;
	ext->dec_point_pos = 0;
	ext->digit_count = 4;
	ext->digit_padding_left = 0;
	ext->step = 1;
	ext->range_max = 9999;
	ext->range_min = -9999;
	ext->drag_value = LV_SPINVAL_NOT_PRESSED;

	lv_ta_set_cursor_type(new_spinval, LV_CURSOR_NONE);
	lv_ta_set_one_line(new_spinval, true);
	//lv_ta_set_scroll_propagation(new_spinval, true);
	lv_ta_set_sb_mode(new_spinval, LV_SB_MODE_OFF);
	lv_ta_set_text_align(new_spinval, LV_LABEL_ALIGN_CENTER);


//    lv_label_set_long_mode(ext->ta.label, LV_LABEL_LONG_CROP);
//    lv_page_set_scrl_fit2(new_spinval, LV_FIT_TIGHT, LV_FIT_TIGHT);
//    lv_label_set_align(ext->ta.label, LV_LABEL_ALIGN_CENTER);

    //lv_obj_set_width(ext->ta.label, lv_page_get_fit_width(new_spinval)/2);

	//lv_ta_set_cursor_click_pos(new_spinval, false);

	/*The signal and design functions are not copied so set them here*/
	lv_obj_set_signal_cb(new_spinval, lv_spinval_signal);
	lv_obj_set_signal_cb(lv_page_get_scrl(new_spinval), lv_spinval_scrl_signal);
	lv_obj_set_design_cb(new_spinval, ancestor_design); /*Leave the Text area's design function*/

	/*Init the new spinval spinval*/
	if (copy == NULL) {
		/*Set the default styles*/
		lv_theme_t *th = lv_theme_get_current();
		if (th) {
			lv_spinval_set_style(new_spinval, LV_SPINVAL_STYLE_BG,
					th->style.spinval.bg);
			lv_spinval_set_style(new_spinval, LV_SPINVAL_STYLE_CURSOR,
					th->style.spinval.cursor);
			lv_spinval_set_style(new_spinval, LV_SPINVAL_STYLE_SB,
					th->style.spinval.sb);
		}
	}
	/*Copy an existing spinval*/
	else {
		lv_spinval_ext_t *copy_ext = lv_obj_get_ext_attr(copy);

		lv_spinval_set_value(new_spinval, copy_ext->value);
		lv_spinval_set_digit_format(new_spinval, copy_ext->digit_count,
				copy_ext->dec_point_pos);
		lv_spinval_set_range(new_spinval, copy_ext->range_min,
				copy_ext->range_max);
		lv_spinval_set_step(new_spinval, copy_ext->step);

		/*Refresh the style with new signal function*/
		lv_obj_refresh_style(new_spinval);
	}

	lv_spinval_updatevalue(new_spinval);

	LV_LOG_INFO("spinval created");

	return new_spinval;
}

/*=====================
 * Setter functions
 *====================*/

/**
 * Set spinval value
 * @param spinval pointer to spinval
 * @param i value to be set
 */
void lv_spinval_set_value(lv_obj_t *spinval, int32_t i) {
	lv_spinval_ext_t *ext = lv_obj_get_ext_attr(spinval);
	if (ext == NULL)
		return;

	if (i > ext->range_max)
		i = ext->range_max;
	if (i < ext->range_min)
		i = ext->range_min;

	ext->value = i;

	lv_spinval_updatevalue(spinval);
}

void lv_spinval_set_text(lv_obj_t *spinval, int32_t i, const char * text) {
	lv_spinval_ext_t *ext = lv_obj_get_ext_attr(spinval);
	if (ext == NULL)
		return;

	if (i > ext->range_max)
		i = ext->range_max;
	if (i < ext->range_min)
		i = ext->range_min;

	ext->value = i;

	lv_spinval_updatetext(spinval, text);
}

/**
 * Set spinval digit format (digit count and decimal format)
 * @param spinval pointer to spinval
 * @param digit_count number of digit excluding the decimal separator and the sign
 * @param separator_position number of digit before the decimal point. If 0, decimal point is not
 * shown
 */
void lv_spinval_set_digit_format(lv_obj_t *spinval, uint8_t digit_count,
		uint8_t separator_position) {
	lv_spinval_ext_t *ext = lv_obj_get_ext_attr(spinval);
	if (ext == NULL)
		return;

	if (digit_count > LV_SPINVAL_MAX_DIGIT_COUNT)
		digit_count = LV_SPINVAL_MAX_DIGIT_COUNT;

	if (separator_position > LV_SPINVAL_MAX_DIGIT_COUNT)
		separator_position = LV_SPINVAL_MAX_DIGIT_COUNT;

	ext->digit_count = digit_count;
	ext->dec_point_pos = separator_position;

	lv_spinval_updatevalue(spinval);
}

/**
 * Set spinval step
 * @param spinval pointer to spinval
 * @param step steps on increment/decrement
 */
void lv_spinval_set_step(lv_obj_t *spinval, uint32_t step) {
	lv_spinval_ext_t *ext = lv_obj_get_ext_attr(spinval);
	if (ext == NULL)
		return;

	ext->step = step;
}

/**
 * Set spinval value range
 * @param spinval pointer to spinval
 * @param range_min maximum value, inclusive
 * @param range_max minimum value, inclusive
 */
void lv_spinval_set_range(lv_obj_t *spinval, int32_t range_min,
		int32_t range_max) {
	lv_spinval_ext_t *ext = lv_obj_get_ext_attr(spinval);
	if (ext == NULL)
		return;

	ext->range_max = range_max;
	ext->range_min = range_min;

	if (ext->value > ext->range_max) {
		ext->value = ext->range_max;
		lv_obj_invalidate(spinval);
	}
	if (ext->value < ext->range_min) {
		ext->value = ext->range_min;
		lv_obj_invalidate(spinval);
	}
}

/**
 * Set spinval left padding in digits count (added between sign and first digit)
 * @param spinval pointer to spinval
 * @param cb Callback function called on value change event
 */
void lv_spinval_set_padding_left(lv_obj_t *spinval, uint8_t padding) {
	lv_spinval_ext_t *ext = lv_obj_get_ext_attr(spinval);
	ext->digit_padding_left = padding;
	lv_spinval_updatevalue(spinval);
}

/*=====================
 * Getter functions
 *====================*/

/**
 * Get the spinval numeral value (user has to convert to float according to its digit format)
 * @param spinval pointer to spinval
 * @return value integer value of the spinval
 */
int32_t lv_spinval_get_value(lv_obj_t *spinval) {
	lv_spinval_ext_t *ext = lv_obj_get_ext_attr(spinval);

	return ext->value;
}

/*=====================
 * Other functions
 *====================*/

/**
 * Select next lower digit for edition
 * @param spinval pointer to spinval
 */
void lv_spinval_step_next(lv_obj_t *spinval) {
	lv_spinval_ext_t *ext = lv_obj_get_ext_attr(spinval);

	int32_t new_step = ext->step / 10;
	if ((new_step) > 0)
		ext->step = new_step;
	else
		ext->step = 1;

	lv_spinval_updatevalue(spinval);
}

/**
 * Select next higher digit for edition
 * @param spinval pointer to spinval
 */
void lv_spinval_step_prev(lv_obj_t *spinval) {
	lv_spinval_ext_t *ext = lv_obj_get_ext_attr(spinval);
	int32_t step_limit;
	step_limit = LV_MATH_MAX(ext->range_max,
			(ext->range_min < 0 ? (-ext->range_min) : ext->range_min));
	int32_t new_step = ext->step * 10;
	if (new_step <= step_limit)
		ext->step = new_step;

	lv_spinval_updatevalue(spinval);
}

/**
 * Increment spinval value by one step
 * @param spinval pointer to spinval
 */
void lv_spinval_increment(lv_obj_t *spinval) {
	lv_spinval_ext_t *ext = lv_obj_get_ext_attr(spinval);
	//printf("spinval inc\n");

	if (ext->value + ext->step <= ext->range_max) {
		/*Special mode when zero crossing*/
		if ((ext->value + ext->step) > 0 && ext->value < 0)
			ext->value = -ext->value;
		ext->value += ext->step;

	} else {
		ext->value = ext->range_max;
	}

	lv_spinval_updatevalue(spinval);
}

/**
 * Decrement spinval value by one step
 * @param spinval pointer to spinval
 */
void lv_spinval_decrement(lv_obj_t *spinval) {
	lv_spinval_ext_t *ext = lv_obj_get_ext_attr(spinval);
	//printf("spinval dec\n");
	if (ext->value - ext->step >= ext->range_min) {
		/*Special mode when zero crossing*/
		if ((ext->value - ext->step) < 0 && ext->value > 0)
			ext->value = -ext->value;
		ext->value -= ext->step;
	} else {
		ext->value = ext->range_min;
	}

	lv_spinval_updatevalue(spinval);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
static lv_res_t lv_spinval_scrl_signal(lv_obj_t *textarea, lv_signal_t sign,
		void *param) {

	lv_res_t res;
    /* Include the ancient signal function */
	res = ancestor_scrl_signal(textarea, sign, param);
    if(res != LV_RES_OK) return res;

	lv_obj_t *spinval = lv_obj_get_parent(textarea);
	lv_spinval_ext_t *ext = lv_obj_get_ext_attr(spinval);
	const lv_style_t *spinval_style = lv_obj_get_style(spinval);

	if (sign == LV_SIGNAL_PRESSING) {

			lv_point_t p;
			lv_coord_t h = LV_VER_RES_MAX / 2;  // 240
			lv_indev_get_point(param, &p);
			p.y += lv_obj_get_y(spinval);
			p.y = (LV_VER_RES_MAX - p.y) + ext->range_min;

			int16_t tmp = 0;

			tmp = (int32_t) ((int32_t) p.y * (ext->range_max - ext->range_min + 1)) / h;

			//tmp = (int32_t) ((int32_t) p.y * (ext->range_max - ext->range_min + 1)) / h;
			//printf("\ntmp1:%d  y:%d\n", tmp, p.y);
			//tmp = ext->range_max - tmp; /*Invert the value: smaller value means higher y*/
			//printf("tmp2:%d\n", tmp);

			if (tmp < ext->range_min)
			{
				tmp = ext->range_min;
			}
			else if (tmp > ext->range_max)
			{
				tmp = ext->range_max;
			}

			//printf("tmp3:%d\n", tmp);
			lv_spinval_set_value(spinval, tmp);
//			if (tmp > ext->drag_value) {
//				ext->drag_value = tmp;
//				lv_spinval_increment(spinval);
//			} else if (tmp < ext->drag_value) {
//				ext->drag_value = tmp;
//				lv_spinval_decrement(spinval);
//			}

		}
	return res;
}
/**
 * Signal function of the spinval
 * @param spinval pointer to a spinval object
 * @param sign a signal type from lv_signal_t enum
 * @param param pointer to a signal specific variable
 * @return LV_RES_OK: the object is not deleted in the function; LV_RES_INV: the object is deleted
 */
static lv_res_t lv_spinval_signal(lv_obj_t *spinval, lv_signal_t sign,
		void *param) {
	//LV_LOG_TRACE("spinval signal");
	lv_spinval_ext_t *ext = lv_obj_get_ext_attr(spinval);

	lv_res_t res = LV_RES_OK;

	/* Include the ancient signal function */
    if(sign != LV_SIGNAL_CONTROL) {
        res = ancestor_signal(spinval, sign, param);
        if(res != LV_RES_OK) return res;
    }
    else if (sign == LV_SIGNAL_CLEANUP) {
		/*Nothing to cleanup. (No dynamically allocated memory in 'ext')*/
	} else if (sign == LV_SIGNAL_GET_TYPE) {
		lv_obj_type_t *buf = param;
		uint8_t i;
		for (i = 0; i < LV_MAX_ANCESTOR_NUM - 1; i++) { /*Find the last set data*/
			if (buf->type[i] == NULL)
				break;
		}
		buf->type[i] = "lv_spinval";
	}

	return res;
}

static void lv_spinval_updatevalue(lv_obj_t *spinval) {
	lv_spinval_ext_t *ext = lv_obj_get_ext_attr(spinval);

	char buf[LV_SPINVAL_MAX_DIGIT_COUNT + 8];
	memset(buf, 0, sizeof(buf));
	char *buf_p = buf;

	/*Add the sign*/
	(*buf_p) = ext->value >= 0 ? ' ' : '-';
	buf_p++;

	int i;
	/*padding left*/
	for (i = 0; i < ext->digit_padding_left; i++) {
		(*buf_p) = ' ';
		buf_p++;
	}

	char digits[64];
	/*Convert the numbers to string (the sign is already handled so always covert positive number)*/
	lv_utils_num_to_str(ext->value < 0 ? -ext->value : ext->value, digits);

	/*Add leading zeros*/
	int lz_cnt = ext->digit_count - (int) strlen(digits);
	if (lz_cnt > 0) {
//		for (i = strlen(digits); i >= 0; i--) {
//			digits[i + lz_cnt] = digits[i];
//		}
//		for (i = 0; i < lz_cnt; i++) {
//        	if(i == lz_cnt)
//        	{
//        		if(ext->value > 0)
//        		{
//        			//digits[i - 1] = ' ';
//        		}
//        		else if(ext->value == 0)
//        		{
//
//        		}
//        		else
//        		{
//        			//digits[i - 1] = '-';
//        		}
//        	}
//        	else
//        	{
//			  digits[i] = ' ';
//        	}
//		}
	}

	int32_t intDigits;
	intDigits =
			(ext->dec_point_pos == 0) ? ext->digit_count : ext->dec_point_pos;

	/*Add the decimal part*/
	for (i = 0; i < intDigits && digits[i] != '\0'; i++) {
		(*buf_p) = digits[i];
		buf_p++;
	}

	if (ext->dec_point_pos != 0) {
		/*Insert the decimal point*/
		(*buf_p) = '.';
		buf_p++;

		for (/*Leave i*/; i < ext->digit_count && digits[i] != '\0'; i++) {
			(*buf_p) = digits[i];
			buf_p++;
		}
	}

	/*Refresh the text*/
	lv_ta_set_text(spinval, (char*) buf);

//	/*Set the cursor position*/
//	int32_t step = ext->step;
//	uint8_t cur_pos = ext->digit_count;
//	while (step >= 10) {
//		step /= 10;
//		cur_pos--;
//	}
//
//	if (cur_pos > intDigits)
//		cur_pos++; /*Skip teh decimal point*/
//
//	cur_pos += ext->digit_padding_left;

	//lv_ta_set_cursor_pos(spinval, cur_pos);
}

static void lv_spinval_updatetext(lv_obj_t *spinval, const char * text) {
	lv_spinval_ext_t *ext = lv_obj_get_ext_attr(spinval);

	char buf[LV_SPINVAL_MAX_DIGIT_COUNT + 8];
	memset(buf, 0, sizeof(buf));
	char *buf_p = buf;

	int i;

	for (i = 0; i < (ext->digit_count) && text[i] != '\0'; i++) {
		(*buf_p) = text[i];
		buf_p++;
	}

	/*Refresh the text*/
	lv_ta_set_text(spinval, (char*) buf);

}

#endif
