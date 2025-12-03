#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".lv_ui_core.data.bss")
#pragma data_seg(".lv_ui_core.data")
#pragma const_seg(".lv_ui_core.text.const")
#pragma code_seg(".lv_ui_core.text")
#endif

/**
 * @file lv_math.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_math.h"
#if LV_MATH_USE_HW
#include "math_fast_function.h"
#endif

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
#if !LV_MATH_USE_HW_SIN
static const int16_t sin0_90_table[] = {
    0,     572,   1144,  1715,  2286,  2856,  3425,  3993,  4560,  5126,  5690,  6252,  6813,  7371,  7927,  8481,
    9032,  9580,  10126, 10668, 11207, 11743, 12275, 12803, 13328, 13848, 14364, 14876, 15383, 15886, 16383, 16876,
    17364, 17846, 18323, 18794, 19260, 19720, 20173, 20621, 21062, 21497, 21925, 22347, 22762, 23170, 23571, 23964,
    24351, 24730, 25101, 25465, 25821, 26169, 26509, 26841, 27165, 27481, 27788, 28087, 28377, 28659, 28932, 29196,
    29451, 29697, 29934, 30162, 30381, 30591, 30791, 30982, 31163, 31335, 31498, 31650, 31794, 31927, 32051, 32165,
    32269, 32364, 32448, 32523, 32587, 32642, 32687, 32722, 32747, 32762, 32767
};
#endif
/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Return with sinus of an angle
 * @param angle
 * @return sinus of 'angle'. sin(-90) = -32767, sin(90) = 32767
 */
int16_t LV_ATTRIBUTE_FAST_MEM lv_trigo_sin(int16_t angle)
{
#if !LV_MATH_USE_HW_SIN
    int16_t ret = 0;
    angle       = angle % 360;

    if (angle < 0) {
        angle = 360 + angle;
    }

    if (angle < 90) {
        ret = sin0_90_table[angle];
    } else if (angle >= 90 && angle < 180) {
        angle = 180 - angle;
        ret   = sin0_90_table[angle];
    } else if (angle >= 180 && angle < 270) {
        angle = angle - 180;
        ret   = -sin0_90_table[angle];
    } else { /*angle >=270*/
        angle = 360 - angle;
        ret   = -sin0_90_table[angle];
    }
    return ret;
#else
    // 规范化角度
    angle = angle % 360;
    if (angle < 0) {
        angle += 360;
    }

    // 特殊角度优化
    if (angle == 0) {
        return 0;
    }
    if (angle == 90) {
        return 32767;
    }
    if (angle == 180) {
        return 0;
    }
    if (angle == 270) {
        return -32767;
    }

    // 转换为弧度（定点数）
    long angle_rad_fixed = (long)(angle * DEG_TO_RAD_FACTOR);

    // 调用硬件加速
    long sin_fixed = sin_fix(angle_rad_fixed);

    // 转换为 LVGL 格式
    int32_t result = ((int64_t)sin_fixed * 32767) >> 29;

    // 确保在范围内
    if (result > 32767) {
        return 32767;
    }
    if (result < -32767) {
        return -32767;
    }

    return (int16_t)result;
#endif
}

/**
 * Calculate a value of a Cubic Bezier function.
 * @param t time in range of [0..LV_BEZIER_VAL_MAX]
 * @param u0 start values in range of [0..LV_BEZIER_VAL_MAX]
 * @param u1 control value 1 values in range of [0..LV_BEZIER_VAL_MAX]
 * @param u2 control value 2 in range of [0..LV_BEZIER_VAL_MAX]
 * @param u3 end values in range of [0..LV_BEZIER_VAL_MAX]
 * @return the value calculated from the given parameters in range of [0..LV_BEZIER_VAL_MAX]
 */
uint32_t lv_bezier3(uint32_t t, uint32_t u0, uint32_t u1, uint32_t u2, uint32_t u3)
{
    uint32_t t_rem  = 1024 - t;
    uint32_t t_rem2 = (t_rem * t_rem) >> 10;
    uint32_t t_rem3 = (t_rem2 * t_rem) >> 10;
    uint32_t t2     = (t * t) >> 10;
    uint32_t t3     = (t2 * t) >> 10;

    uint32_t v1 = (t_rem3 * u0) >> 10;
    uint32_t v2 = (3 * t_rem2 * t * u1) >> 20;
    uint32_t v3 = (3 * t_rem * t2 * u2) >> 20;
    uint32_t v4 = (t3 * u3) >> 10;

    return v1 + v2 + v3 + v4;
}

/**
 * Get the square root of a number
 * @param x integer which square root should be calculated
 * @param q store the result here. q->i: integer part, q->f: fractional part in 1/256 unit
 * @param mask optional to skip some iterations if the magnitude of the root is known.
 * Set to 0x8000 by default.
 * If root < 16: mask = 0x80
 * If root < 256: mask = 0x800
 * Else: mask = 0x8000
 */
void LV_ATTRIBUTE_FAST_MEM lv_sqrt(uint32_t x, lv_sqrt_res_t *q, uint32_t mask)
{
#if !LV_MATH_USE_HW_SQRT

    x = x << 8; /*To get 4 bit precision. (sqrt(256) = 16 = 4 bit)*/

    uint32_t root = 0;
    uint32_t trial;
    // http://ww1.microchip.com/...en/AppNotes/91040a.pdf
    do {
        trial = root + mask;
        if (trial * trial <= x) {
            root = trial;
        }
        mask = mask >> 1;
    } while (mask);

    q->i = root >> 4;
    q->f = (root & 0xf) << 4;
#else
    if (x == 0) {
        q->i = 0;
        q->f = 0;
        return;
    }

    struct data_q_struct input = {
        .data = (long)x,  // 输入值
        .q = 0            // 输入为整数（Q0格式）
    };

    struct data_q_struct result = root_fix(input);

    const int RESULT_Q = 16;
    long fixed_result = result.data;

    q->i = (uint32_t)(fixed_result >> RESULT_Q);          // 整数部分
    q->f = (uint32_t)(fixed_result & 0xFFFF); // 小数部分

    if (mask != 0) {
        // 应用 mask 到结果
        q->f &= mask;
    }
    // 精度补偿
    if (q->f > 0 && (uint32_t)q->i * q->i > x) {
        q->f = 0;
        if (q->i > 0) {
            q->i--;
        }
    }
#endif
}

/**
 * Calculate the atan2 of a vector.
 * @param x
 * @param y
 * @return the angle in degree calculated from the given parameters in range of [0..360]
 */
uint16_t lv_atan2(int x, int y)
{
    // Fast XY vector to integer degree algorithm - Jan 2011 www.RomanBlack.com
    // Converts any XY values including 0 to a degree value that should be
    // within +/- 1 degree of the accurate value without needing
    // large slow trig functions like ArcTan() or ArcCos().
    // NOTE! at least one of the X or Y values must be non-zero!
    // This is the full version, for all 4 quadrants and will generate
    // the angle in integer degrees from 0-360.
    // Any values of X and Y are usable including negative values provided
    // they are between -1456 and 1456 so the 16bit multiply does not overflow.

#if !LV_MATH_USE_HW_ARCTAN
    unsigned char negflag;
    unsigned char tempdegree;
    unsigned char comp;
    unsigned int degree;     // this will hold the result
    unsigned int ux;
    unsigned int uy;

    // Save the sign flags then remove signs and get XY as unsigned ints
    negflag = 0;
    if (x < 0) {
        negflag += 0x01;    // x flag bit
        x = (0 - x);        // is now +
    }
    ux = x;                // copy to unsigned var before multiply
    if (y < 0) {
        negflag += 0x02;    // y flag bit
        y = (0 - y);        // is now +
    }
    uy = y;                // copy to unsigned var before multiply

    // 1. Calc the scaled "degrees"
    if (ux > uy) {
        degree = (uy * 45) / ux;   // degree result will be 0-45 range
        negflag += 0x10;    // octant flag bit
    } else {
        degree = (ux * 45) / uy;   // degree result will be 0-45 range
    }

    // 2. Compensate for the 4 degree error curve
    comp = 0;
    tempdegree = degree;    // use an unsigned char for speed!
    if (tempdegree > 22) {   // if top half of range
        if (tempdegree <= 44) {
            comp++;
        }
        if (tempdegree <= 41) {
            comp++;
        }
        if (tempdegree <= 37) {
            comp++;
        }
        if (tempdegree <= 32) {
            comp++;    // max is 4 degrees compensated
        }
    } else { // else is lower half of range
        if (tempdegree >= 2) {
            comp++;
        }
        if (tempdegree >= 6) {
            comp++;
        }
        if (tempdegree >= 10) {
            comp++;
        }
        if (tempdegree >= 15) {
            comp++;    // max is 4 degrees compensated
        }
    }
    degree += comp;   // degree is now accurate to +/- 1 degree!

    // Invert degree if it was X>Y octant, makes 0-45 into 90-45
    if (negflag & 0x10) {
        degree = (90 - degree);
    }

    // 3. Degree is now 0-90 range for this quadrant,
    // need to invert it for whichever quadrant it was in
    if (negflag & 0x02) { // if -Y
        if (negflag & 0x01) { // if -Y -X
            degree = (180 + degree);
        } else {    // else is -Y +X
            degree = (180 - degree);
        }
    } else { // else is +Y
        if (negflag & 0x01) { // if +Y -X
            degree = (360 - degree);
        }
    }
    return degree;
#else
    // 处理特殊点
    if (x == 0 && y == 0) {
        return 0;
    }
    if (x == 0) {
        return (y > 0) ? 90 : 270;
    }
    if (y == 0) {
        return (x > 0) ? 0 : 180;
    }

    // 调用硬件加速函数
    struct data_q_struct result = angle_fix((long)x, (long)y);

    // 提取角度值
    const int RESULT_Q = 16;
    long fixed_result = result.data;

    // 转换为整数角度 (0-360)
    uint32_t full_angle = (uint32_t)(fixed_result >> RESULT_Q);

    // 规范化到 0-360 范围
    if (full_angle >= 360) {
        full_angle %= 360;
    }

    return (uint16_t)full_angle;
#endif
}

/**
 * Calculate the integer exponents.
 * @param base
 * @param power
 * @return base raised to the power exponent
 */
int64_t lv_pow(int64_t base, int8_t exp)
{
    int64_t result = 1;
    while (exp) {
        if (exp & 1) {
            result *= base;
        }
        exp >>= 1;
        base *= base;
    }

    return result;
}

/**
 * Get the mapped of a number given an input and output range
 * @param x integer which mapped value should be calculated
 * @param min_in min input range
 * @param max_in max input range
 * @param min_out max output range
 * @param max_out max output range
 * @return the mapped number
 */
int32_t lv_map(int32_t x, int32_t min_in, int32_t max_in, int32_t min_out, int32_t max_out)
{
    if (max_in >= min_in && x >= max_in) {
        return max_out;
    }
    if (max_in >= min_in && x <= min_in) {
        return min_out;
    }

    if (max_in <= min_in && x <= max_in) {
        return max_out;
    }
    if (max_in <= min_in && x >= min_in) {
        return min_out;
    }

    /**
     * The equation should be:
     *   ((x - min_in) * delta_out) / delta in) + min_out
     * To avoid rounding error reorder the operations:
     *   (x - min_in) * (delta_out / delta_min) + min_out
     */

    int32_t delta_in = max_in - min_in;
    int32_t delta_out = max_out - min_out;

    return ((x - min_in) * delta_out) / delta_in + min_out;
}

#include "cpu.h"
uint32_t lv_rand(uint32_t min, uint32_t max)
{
    return (rand32() % (max - min + 1)) + min;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
