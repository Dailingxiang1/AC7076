#include "app_config.h"
/* #include "app_task.h" */
#include "system/timer.h"
#include "device/device.h"
#include "key_event_deal.h"
#include "asm/math_fast_function.h"
#include "res/resfile.h"
#include "jlui/ui.h"
#include "jlui_app/ui_style.h"
#include "jlui_app/ui_api.h"
#include "jlui_app/res_config.h"
#include "jlui_app/ui_resource.h"
#include "jlui_app/ui_sys_param.h"
#include "jlui_app/ui_menu_manage.h"

#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI_MENU]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_menu_waterfalls.data.bss")
#pragma data_seg(".ui_action_menu_waterfalls.data")
#pragma const_seg(".ui_action_menu_waterfalls.text.const")
#pragma code_seg(".ui_action_menu_waterfalls.text")
#endif

#ifdef CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE
#if TCFG_UI_MENU_WATERFALLS_ENABLE

#define STYLE_NAME  JL
/*##############################################################
                //蜂窝滚筒菜单版本说明

*初版功能
##############################################################*/

#define STAR_RATIO_MODE_SPHERE_FUNC 1
#define STAR_RATIO_MODE_LINEAR_FUNC 2
#define STAR_RATIO_MODE_EXP_FUNC    3
/*##############################################################
                //蜂窝滚筒菜单效果配置区
##############################################################*/
//#define STAR_LAYOUT             STAR_LAYOUT    //满天星布局
#define STAR_LAYOUT				MENU_LAYOUT_STAR
#define ICON_DEBUG              0               //开发板调试使用

#define S_LCD_WIDTH             320
#define S_LCD_HEIGHT            386

#define ICON_WIDTH_HEIGHT_MAX   80              //控件宽高
#define ICON_INTERVAL_MAX       10               //控件间隔

#define SQUARE_SCREEN           1               //方屏使能
#define ROUNDED_RECTANGLE_R     100              //方屏圆角矩形半径，影响图标到四角的变化

#define MODE_2_5                1               //球面效果

/* #define	SPHERE_R				300             //球体半径,需大于屏幕对角线半径 */
#define STAR_ROTATE_MODE        (STAR_RATIO_MODE_LINEAR_FUNC)  //缩放曲线

#define	SPHERE_R				300             //RATIO_MODE_SPHERE_FUNC时，表示球体半径,需大于屏幕对角线半径
#define LINEAR_FUNC_A           630             //RATIO_MODE_LINEAR_FUNC参数

#define SPHERE_DIST_A           40              //球体图标间距系数,越小图标越发散，过大会造成图标向中心重叠
#define SPHERE_DIST_B			20				//越大图标越发散


#define root3                   1.732f          //√3
#define root3_half              0.866f          //√3/2

#define AUTO_SEL_INDEX          0               //自动切换图片索引

#define VAILD_ICON_NUM				28
#define LOOP_ENABLE				1				//瀑布菜单循环滚动
#define LOOP_START				7				//默认
#define LOOP_DISTANCE			180				//第一行图标移动到第三行的步进(每两行为一组)
/*##############################################################
                //工具函数
##############################################################*/
//#define PI              3.14159f
//#define complex_abs_float(x,y)  (sqrt((float)((float)x)*((float)x)+((float)y)*((float)y)))
//#define complex_dqdt_float(x,y) (sqrt((float)((float)x)*((float)x)-((float)y)*((float)y)))
//#define root_float(x)           (sqrt((float)x))
//#define sin_float(x)            (sin(PI*(float)x))
//#define cos_float(x)            (cos(PI*(float)x))

#define CSS(x,X)        (x)             //绝对坐标转相对坐标
#define CSS2ABS(x,X)    (x)             //相对坐标转绝对坐标
#define C2LT(X,WH)      ((X)-(WH/2))                //图标中心转左上角坐标
#define LT2C(X,WH)      ((X)+(WH/2))                //图标左上角转中心坐标
#define ABS(x)          ((x)>0?(x):(-(x)))          //绝对值
/* #define ABS(x)  		__builtin_abs(x) */
#define DIR(x)          ((x)>0?(1):(-1))            //方向
/* #define MIN(a,b)        ((a<b)?(a):(b))             //最小值 */

//圆角矩形方位
enum {
    ROUNDED_RECT_NULL,
    ROUNDED_RECT_LEFT_TOP,
    ROUNDED_RECT_RIGHT_TOP,
    ROUNDED_RECT_LEFT_BOTTOM,
    ROUNDED_RECT_RIGHT_BOTTOM,
};

#if ICON_DEBUG
#define SCREEN_LEFT         80
#define SCREEN_TOP          50
#define SCREEN_RIGHT        374
#define SCREEN_BOTTOM       404
#else
#define SCREEN_LEFT         0
#define SCREEN_TOP          0
#define SCREEN_RIGHT        S_LCD_WIDTH
#define SCREEN_BOTTOM       S_LCD_HEIGHT
#endif

static int icon_postion(struct layout *__layout, u8 ratio_index, int x_offset, int y_offset, int bound_en, int redraw);

const s16 waterfalls_center_y_tab[] = {200, 110, 20, -70, -160, -250, -340, -430};
/* const s16 waterfalls_flick_y_tab[]  = {360,570}; */
/*##############################################################
                //蜂窝菜单效果信息
##############################################################*/
struct waterfalls_anim {
    ui_anim_t anim;			//动画句柄
    int dist_x;				//移动距离记录
    int dist_y;				//移动距离记录
    int tanx;				//x与y方向的比值
    int only_x_en;				//只有x方向使能
    float *icon_ratio_tab;	//缓存缩放等级
};
struct icon_info { 				//图标信息
    struct position icon_center;        //中心坐标
    void *ctrl;
    u8 hold_flag;
    u8 down_flag;
#if LOOP_ENABLE
    int start_idx;
#endif
    struct waterfalls_anim *p_anim;
};

static struct icon_info __icon_info;
#define __this 		(&__icon_info)

struct star_info {
    u8 index;           //索引
    u8 icon_level: 7;   //圈层
    u8 invisible: 1;    //是否隐藏
    int ic_x;           //图标中心点坐标
    int ic_y;           //图标中心点坐标
    int width;          //图标缩放后的宽
    int height;         //图标缩放后的高
    float ratio;        //缩放系数
};

//位于屏幕中间位置的图标的坐标
#define CENTER_ICON_X		120
#define CENTER_ICON_Y		153

/******************************************************************
                满天星滚桶算法
* __info : 图标信息
* total_ratio ：整体缩放系数，用于编码器调节
* center_x 中心图标位置
* center_y 中心图标位置
* icon_index 图标序号
* bound_en 边框约束
******************************************************************/
static int star_icon_info_do(struct star_info *__info, float total_ratio, int center_x, int center_y, int icon_index, int bound_en)
{
    //获取屏幕尺寸信息
    int lcd_width = S_LCD_WIDTH;
    int lcd_height = S_LCD_HEIGHT;
    int lcd_width_half = lcd_width / 2;
    int lcd_height_half = lcd_height / 2;
    //获取图标信息
    int icon_width_height = ICON_WIDTH_HEIGHT_MAX;
    int icon_distance = icon_width_height + ICON_INTERVAL_MAX;
    int icon_distance_r = icon_distance * total_ratio;
    int icon_distance_r_half = icon_distance_r / 2;

    float x_abs, y_abs;                         //位置参数abs
    double ratio_val = 1.0f;                           //缩放系数
    float triangle_a, triangle_b, triangle_c;   //当前图标、圈层首图标、屏幕中心维成三角形的三边长
    float angle_offset;                         //偏移角度 按60度差偏移得到全屏坐标
    float sin_angle, cos_angle;                 //与水平轴夹角正余弦值（0-60°）
    float sin_offset, cos_offset;               //与水平轴夹角正余弦值（0-360°）
    float dist_x, dist_y, dist_c;               //图标中心到屏幕中心的水平、垂直和径向距离
    float dist_c_big, dist_c_small;             //图标所在圆与屏幕中心的最大和最小距离
    int icon_abs_left, icon_abs_right, icon_abs_top, icon_abs_bottom;//图标边界与中心轴的距离
    int icon_level = 0;                             //图标圈层
    int real_index = 0;
    int screen_lock = 0;          //2.5D投影时，隐藏z负轴球面图标
    int invisible = 0;
    //计算图标分组
    int group_index = icon_index / 7;             //组序
    int group_in_index = icon_index % 7;          //组内序
    if (group_in_index < 3) {
        x_abs = lcd_width_half + (group_in_index - 1) * icon_distance_r;
        y_abs = 0;
    } else {
        x_abs = lcd_width_half - icon_distance_r_half + ((group_in_index - 3) - 1) * icon_distance_r;
        y_abs = (float)root3_half * icon_distance_r;
    }
    y_abs += (float)group_index * root3 * icon_distance_r;
    //图标的中心坐标
    y_abs += (float)center_y;                   //叠加整体偏移
    x_abs += (float)center_x;                   //叠加整体偏移
    dist_x = (x_abs - lcd_width_half);          //到屏幕中心的水平距离
    dist_y = (y_abs - lcd_height_half);         //到屏幕中心的垂直距离
    dist_c = complex_abs_float(dist_x, dist_y); //到屏幕中心的径向距离
#if MODE_2_5        //2.5D立体效果
    /*
        简化的球面投影模型
    */
#if (STAR_ROTATE_MODE == STAR_RATIO_MODE_SPHERE_FUNC)
    if (SPHERE_R > dist_c) {
        float cos_z_angle = (float)complex_dqdt_float(SPHERE_R, dist_c) / SPHERE_R;             //计算与z轴夹角余弦值
#elif (STAR_ROTATE_MODE == STAR_RATIO_MODE_LINEAR_FUNC)
    if (LINEAR_FUNC_A / 2 > dist_c) {
        float cos_z_angle = (float)(LINEAR_FUNC_A - dist_c) / LINEAR_FUNC_A;
#else
#error "ROTATE_MODE not defined"
#endif
        ratio_val = cos_z_angle;
        //重算图标到屏幕中心的径向距离,也就是2.5D下图标的疏密程度
//        float dist_r = ((float) dist_c / SPHERE_R) *((float) dist_c / SPHERE_R) * SPHERE_DIST_A;
        float dist_r = SPHERE_DIST_A * (exp_float((float)dist_c / SPHERE_R) - 1) - SPHERE_DIST_B * sin_float((float)dist_c / SPHERE_R / 2);
//        float dist_r = (float)65*dist_c/SPHERE_R;
        screen_lock = (dist_c > SPHERE_R) ? 1 : 0;          //z轴负球面的图标不显示
        if (dist_c && !screen_lock) {                                       //根据新距离计算坐标值
            x_abs -= dist_r * dist_x / dist_c;
            y_abs -= dist_r * dist_y / dist_c;
        }
        dist_x = (x_abs - lcd_width_half);
        dist_y = (y_abs - lcd_height_half);
        dist_c -= dist_r;
#if (STAR_ROTATE_MODE == STAR_RATIO_MODE_SPHERE_FUNC)
        cos_z_angle = (float)complex_dqdt_float(SPHERE_R, dist_c) / SPHERE_R;                 //重算与z轴夹角余弦值
#elif (STAR_ROTATE_MODE == STAR_RATIO_MODE_LINEAR_FUNC)
        cos_z_angle = (float)(LINEAR_FUNC_A - dist_c) / LINEAR_FUNC_A;                    //重算
#else
#error "ROTATE_MODE not defined"
#endif
        ratio_val = cos_z_angle;                                            //重新获取缩放系数
        //printf("[star]index:%d ratio:%.2f,dist_r:%.2f dist_c:%.2f",icon_index,cos_z_angle,dist_r,dist_c);
    }

#endif//MODE_2_5
    //计算图标中心到屏幕中心的水平、垂直和径向距离
    icon_abs_left = x_abs - icon_distance_r * ratio_val  / 2;
    icon_abs_right = x_abs + icon_distance_r * ratio_val / 2;
    icon_abs_top = y_abs - icon_distance_r * ratio_val  / 2;
    icon_abs_bottom = y_abs + icon_distance_r * ratio_val / 2;

#if SQUARE_SCREEN       //方屏处理
    int rounded_rect_left    = SCREEN_LEFT + ROUNDED_RECTANGLE_R;
    int rounded_rect_right   = SCREEN_RIGHT - ROUNDED_RECTANGLE_R;
    int rounded_rect_top     = SCREEN_TOP + ROUNDED_RECTANGLE_R;
    int rounded_rect_bottom  = SCREEN_BOTTOM - ROUNDED_RECTANGLE_R;
    /*    int is_rounded_rect_mode = (icon_abs_left<=rounded_rect_left&&icon_abs_top<=rounded_rect_top)?ROUNDED_RECT_LEFT_TOP:ROUNDED_RECT_NULL;
            is_rounded_rect_mode = (is_rounded_rect_mode)?is_rounded_rect_mode:(icon_abs_right>=rounded_rect_right&&icon_abs_top<=rounded_rect_top)?ROUNDED_RECT_RIGHT_TOP:ROUNDED_RECT_NULL;
            is_rounded_rect_mode = (is_rounded_rect_mode)?is_rounded_rect_mode:(icon_abs_left<=rounded_rect_left&&icon_abs_bottom>=rounded_rect_bottom)?ROUNDED_RECT_LEFT_BOTTOM:ROUNDED_RECT_NULL;
            is_rounded_rect_mode = (is_rounded_rect_mode)?is_rounded_rect_mode:(icon_abs_right>=rounded_rect_right&&icon_abs_bottom>=rounded_rect_bottom)?ROUNDED_RECT_RIGHT_BOTTOM:ROUNDED_RECT_NULL;
    */
    int is_rounded_rect_mode = (x_abs <= rounded_rect_left && y_abs <= rounded_rect_top) ? ROUNDED_RECT_LEFT_TOP : ROUNDED_RECT_NULL;
    is_rounded_rect_mode = (is_rounded_rect_mode) ? is_rounded_rect_mode : (x_abs >= rounded_rect_right && y_abs <= rounded_rect_top) ? ROUNDED_RECT_RIGHT_TOP : ROUNDED_RECT_NULL;
    is_rounded_rect_mode = (is_rounded_rect_mode) ? is_rounded_rect_mode : (x_abs <= rounded_rect_left && y_abs >= rounded_rect_bottom) ? ROUNDED_RECT_LEFT_BOTTOM : ROUNDED_RECT_NULL;
    is_rounded_rect_mode = (is_rounded_rect_mode) ? is_rounded_rect_mode : (x_abs >= rounded_rect_right && y_abs >= rounded_rect_bottom) ? ROUNDED_RECT_RIGHT_BOTTOM : ROUNDED_RECT_NULL;

    int rect_center_x = 0;
    int rect_center_y = 0;
    //完全超出屏幕边界的 或者在Z轴负半球面的图标，隐藏
    if ((icon_abs_right <= SCREEN_LEFT) || \
        (icon_abs_bottom <= SCREEN_TOP) || \
        (icon_abs_left >= SCREEN_RIGHT) || \
        (icon_abs_top >= SCREEN_BOTTOM) || screen_lock)
    {
        invisible = 1;
        ratio_val = 1.0f;
    }
    //压在屏幕四角圆边界上的，进行二次缩放
    else if (is_rounded_rect_mode)
    {
        switch (is_rounded_rect_mode) {
        case ROUNDED_RECT_LEFT_TOP:
            rect_center_x = rounded_rect_left;
            rect_center_y = rounded_rect_top;
            break;
        case ROUNDED_RECT_RIGHT_TOP:
            rect_center_x = rounded_rect_right;
            rect_center_y = rounded_rect_top;
            break;
        case ROUNDED_RECT_LEFT_BOTTOM:
            rect_center_x = rounded_rect_left;
            rect_center_y = rounded_rect_bottom;
            break;
        case ROUNDED_RECT_RIGHT_BOTTOM:
            rect_center_x = rounded_rect_right;
            rect_center_y = rounded_rect_bottom;
            break;
        }
        dist_x = (x_abs - rect_center_x);
        dist_y = (y_abs - rect_center_y);

        float rect_center_r = complex_abs_float(dist_x, dist_y);

        dist_c_big = rect_center_r + (float)icon_distance_r * ratio_val / 2;
        dist_c_small = rect_center_r - (float)icon_distance_r * ratio_val / 2;

        invisible = 0;
        if (dist_c_big >= ROUNDED_RECTANGLE_R && bound_en) {
            ratio_val = (float)(ROUNDED_RECTANGLE_R - dist_c_small) / icon_distance_r;
            x_abs -= (rect_center_r - (dist_c_small + (ROUNDED_RECTANGLE_R - dist_c_small) / 2)) * dist_x / rect_center_r;
            y_abs -= (rect_center_r - (dist_c_small + (ROUNDED_RECTANGLE_R - dist_c_small) / 2)) * dist_y / rect_center_r;
        } else {
            ratio_val *= 1.0f;
        }
    } else
    {
        invisible = 0;
        float ratio_val_x, ratio_val_y;
        if (icon_abs_left < SCREEN_LEFT && bound_en) {
            ratio_val_x = (float)(icon_abs_right - SCREEN_LEFT) / icon_distance_r;
            x_abs = SCREEN_LEFT + (icon_abs_right - SCREEN_LEFT) / 2;
        } else if (icon_abs_right > SCREEN_RIGHT && bound_en) {
            ratio_val_x = (float)(SCREEN_RIGHT - icon_abs_left) / icon_distance_r;
            x_abs = SCREEN_RIGHT - (SCREEN_RIGHT - icon_abs_left) / 2;
        } else {
            ratio_val_x = ratio_val;
        }
        if (icon_abs_top < SCREEN_TOP && bound_en) {
            ratio_val_y = (float)(icon_abs_bottom - SCREEN_TOP) / icon_distance_r;
            y_abs = SCREEN_TOP + (icon_abs_bottom - SCREEN_TOP) / 2;
        } else if (icon_abs_bottom > SCREEN_BOTTOM && bound_en) {
            ratio_val_y = (float)(SCREEN_BOTTOM - icon_abs_top) / icon_distance_r;
            y_abs = SCREEN_BOTTOM - (SCREEN_BOTTOM - icon_abs_top) / 2;
        } else {
            ratio_val_y = ratio_val;
        }
        ratio_val = MIN(ratio_val_x, ratio_val_y);
    }
#else   //圆屏处理
    dist_c_big = dist_c + ratio_val * icon_distance_r / 2;
    dist_c_small = dist_c - ratio_val * icon_distance_r / 2;

    int screen_r = MIN(lcd_width_half, lcd_height_half);
    if (dist_c_small >= screen_r || screen_lock)
    {
        invisible = 1;
        ratio_val = 1.0f;
    } else
    {
        invisible = 0;
        if (dist_c_big >= screen_r && bound_en) {
            ratio_val = (float)(screen_r - dist_c_small) / icon_distance_r;
            x_abs = lcd_width_half + (float)(dist_c_small + (screen_r - dist_c_small) / 2) * dist_x / dist_c;
            y_abs = lcd_height_half + (float)(dist_c_small + (screen_r - dist_c_small) / 2) * dist_y / dist_c;
        } else {
            ratio_val *= 1.0f;
        }
    }
#endif//SQUARE_SCREEN
    ratio_val *= total_ratio;
    if (ratio_val < 0.125)
    {
        invisible = 1;
    }
    __info->index = icon_index;
    __info->icon_level = icon_level;
    __info->invisible = invisible;
    __info->ic_x = x_abs;
    __info->ic_y = y_abs;
    __info->width = icon_width_height * ratio_val + 1;
    __info->height = icon_width_height * ratio_val + 1;
    __info->ratio = ratio_val;

    /* printf("%s[%d-%d](%d,%d)(%f) inv:%d %d",\ */
    /* __func__,__info->index,__info->icon_level,__info->ic_x,__info->ic_y,__info->ratio,__info->invisible,is_rounded_rect_mode); */
    return 0;
}
/******************************************************************
                图标缩放与坐标初始化
    __layout 图标所在布局
    ratio_index 缩放等级
    x_offset x方向移动距离
    y_offset y方向移动距离
    bound_en 是否对屏幕边框进行约束
******************************************************************/
static int icon_postion(struct layout *__layout, u8 ratio_index, int x_offset, int y_offset, int bound_en, int redraw)
{
    //移动中心图标
    int move_distance = (ICON_WIDTH_HEIGHT_MAX + ICON_INTERVAL_MAX) / 2;
    __this->icon_center.x += x_offset;
    __this->icon_center.y += y_offset;
    //限制左右滑动范围
    __this->icon_center.x  = (__this->icon_center.x < -1 * move_distance) ? -1 * move_distance : __this->icon_center.x;
    __this->icon_center.x  = (__this->icon_center.x > move_distance) ? move_distance : __this->icon_center.x;

    /* printf("<%s> icon_center: %d * %d\n", __func__, __this->icon_center.x, __this->icon_center.y); */
#if  LOOP_ENABLE

    printf("%s %d cy:%d", __func__, __LINE__, __this->icon_center.y);
    if (__this->icon_center.y >= 0) {
        __this->icon_center.y -= LOOP_DISTANCE;
        __this->start_idx += 7;
    } else if (__this->icon_center.y <= (-LOOP_DISTANCE)) {
        __this->icon_center.y += LOOP_DISTANCE;
        __this->start_idx -= 7;
    }
#endif
    //获取屏幕尺寸信息
    int lcd_width = S_LCD_WIDTH;
    int lcd_height = S_LCD_HEIGHT;
    int icon_level;                             //图标圈层
    int icon_index = __this->start_idx;                         //图标索引
    int real_index = 0;

    struct element *elm, *p, *n;                //遍历图标使用
    elm = &__layout->elm;                       //获取布局句柄
    int icon_index_map = 0;
    list_for_each_child_element_safe(p, n, elm) { //遍历所有图标设置坐标和缩放等级
        struct star_info __info;
        //计算各图标与中心图标的坐标关系
        int icon_index_map = icon_index % VAILD_ICON_NUM;
        star_icon_info_do(&__info, 1.1f, __this->icon_center.x, __this->icon_center.y, icon_index_map, bound_en);
        //将计算的数据复制给icon
        p->css.width = CSS(__info.width, lcd_width);
        p->css.height = CSS(__info.height, lcd_height);
        p->css.left = C2LT(CSS(__info.ic_x, lcd_width), p->css.width);
        p->css.top  = C2LT(CSS(__info.ic_y, lcd_height), p->css.height);
        ui_core_set_element_ratio(p, __info.ratio, __info.ratio, true);
        ui_core_set_element_ratio_change_rect(p, true);
        /* p->css.ratio.en = 1; */
        /* p->css.ratio.ratio_h = __info.ratio; */
        /* p->css.ratio.ratio_w = __info.ratio; */
        p->css.invisible = __info.invisible;
        if (VAILD_ICON_NUM && (icon_index >= (VAILD_ICON_NUM +  __this->start_idx))) {
            p->css.invisible = 1;
        }
        //左对齐，图标抖动不明显
        p->css.hori_align = 1;
        //自动切换图标索引
        /* ui_pic_set_image_index((struct ui_pic *)p, icon_index); */
        icon_index++;
    }
    if (redraw) {
        //启动绘制
        ui_core_redraw(elm);
    }

    return 0;
}
/******************************************************************
                图标初始化
******************************************************************/
static int icon_init(void)
{
    int icon_dist = 5 + ICON_INTERVAL_MAX;


#if LOOP_ENABLE
    __this->icon_center.x = 0;
    __this->icon_center.y = icon_dist;
    __this->start_idx = 10000 * 28 + LOOP_START;
    __this->icon_center.y -= LOOP_DISTANCE;
#else
    __this->icon_center.x = 0;
    __this->icon_center.y = icon_dist;
    __this->start_idx = 0;
#endif
    return 0;
}
/******************************************************************
                图标滑动响应
******************************************************************/
static int icon_move(void *_ctrl, struct element_touch_event *e)
{
    //滑动距离
    int x_offset = e->xoffset;
    int y_offset = e->yoffset;
    icon_postion(_ctrl, 0, x_offset, y_offset, 1, 1);
    return true;
}

/******************************************************************
                图标触摸响应
                返回触摸控件id或者index
******************************************************************/
static int icon_touch_up(void *_ctrl, struct element_touch_event *e)
{
    struct element *elm, *p, *n;
    struct rect r;
    elm = _ctrl;
    int index = 0;
    list_for_each_child_element_safe(p, n, elm) {
        if (!p->css.invisible) {
            if (ui_id2type(p->id) == CTRL_TYPE_PIC) {
                ui_core_get_element_abs_rect(p, &r);
                if (in_rect(&r, &e->pos)) {
                    /* return p->id; */
                    return index;
                }
            }
        }
        index ++;
    }
    return -1;
}
/*****************************************************************
  			动画相关
******************************************************************/
static void waterfalls_anim_exec_cb(int var, int32_t v)
{
    if (!__this) {
        return ;
    }
    if (!__this->p_anim) {
        return ;
    }
    int y_offset = (__this->p_anim->only_x_en) ? 0 : v - __this->p_anim->dist_y;
    __this->p_anim->dist_y = v;

    int x_offset = __this->p_anim->tanx * (v - __this->p_anim->dist_x) / 1024;
    if (x_offset) {
        __this->p_anim->dist_x = v;
    }

    icon_postion(__this->ctrl, 0, x_offset, y_offset, 1, 1);
}

static void waterfalls_anim_stop(void)
{
    if (!__this) {
        return ;
    }

    if (__this->p_anim) {
        if (__this->p_anim->icon_ratio_tab) {
            free(__this->p_anim->icon_ratio_tab);
            __this->p_anim->icon_ratio_tab = NULL;
        }
        ui_anim_del(STAR_LAYOUT, NULL);
        free(__this->p_anim);
        __this->p_anim = NULL;
    }
}
static void waterfalls_anim_ready(struct _ui_anim_t *p)
{
    if (__this->p_anim->icon_ratio_tab) {
        free(__this->p_anim->icon_ratio_tab);
        __this->p_anim->icon_ratio_tab = NULL;
    }
    /* printf("%s %d", __func__, __LINE__); */
}
static void waterfalls_anim_start(struct element_touch_event *e)
{
    int start_dist, end_dist;

    // 释放旧的
    waterfalls_anim_stop();

    __this->p_anim = zalloc(sizeof(struct waterfalls_anim));
    ASSERT(__this->p_anim);

    // 动画参数计算
    int dist_x = e->pos.x >> 16;
    int dist_y = e->pos.y >> 16;
    int energy_t0 = (e->pos.x + 1) & 0xffff; //防止div0
    float vx0 = ABS(2 * (float)dist_x / energy_t0);	//水平初速度
    float vy0 = ABS(2 * (float)dist_y / energy_t0); //垂直初速度
    int xdir = e->pos.y & 0xff;						//水平滑动方向
    int ydir = (e->pos.y >> 8) & 0xff;				//垂直滑动方向

    int run_time = vy0 * 30;						//惯性时间

    end_dist = 3 * run_time ;						//y方向的惯性距离
#if LOOP_ENABLE
    end_dist = end_dist / LOOP_DISTANCE	* LOOP_DISTANCE;
    end_dist -= __this->icon_center.y;
#else
    //居中处理
    if (ydir == 2) {
        for (int i = ARRAY_SIZE(waterfalls_center_y_tab) - 1; i >= 0; i--) {
            if (i == 0 || waterfalls_center_y_tab[i] > __this->icon_center.y + end_dist) {
                end_dist =  waterfalls_center_y_tab[i] - __this->icon_center.y;
                break;
            }
        }
    } else {
        for (int i = 0; i < ARRAY_SIZE(waterfalls_center_y_tab); i++) {
            if (i == (ARRAY_SIZE(waterfalls_center_y_tab) - 1) || waterfalls_center_y_tab[i] < __this->icon_center.y - end_dist) {
                end_dist =  waterfalls_center_y_tab[i] - __this->icon_center.y;
                break;
            }
        }
    }
#endif
    //x方向的距离
    int end_dist_x = 0 - __this->icon_center.x;
    //x分量与y分量的比
    if (end_dist) {
        __this->p_anim->tanx = 1024 * end_dist_x / end_dist;
    } else {
        __this->p_anim->only_x_en = 1;
        __this->p_anim->tanx  = 1024;
        end_dist = end_dist_x;
    }
    /* printf("%s %d %d %d %d %f\n", __func__, ydir, end_dist, run_time, end_dist_x, vy0); */
    start_dist = 0;

    /*重新开始配置惯性*/
    ui_anim_init(&__this->p_anim->anim);
    ui_anim_set_var(&__this->p_anim->anim, STAR_LAYOUT);
    ui_anim_set_path_cb(&__this->p_anim->anim, ui_anim_path_ease_out); 	// 过渡效果
    ui_anim_set_exec_cb(&__this->p_anim->anim, waterfalls_anim_exec_cb);		// 运行回调
    ui_anim_set_values(&__this->p_anim->anim, start_dist, end_dist);		// 路径设置
    ui_anim_set_time(&__this->p_anim->anim, run_time);						// 运行时间设置
    ui_anim_start(&__this->p_anim->anim);
}
static void waterfalls_center_anim_start(struct element_touch_event *e)
{
    int start_dist, end_dist;

    // 释放旧的
    waterfalls_anim_stop();

    __this->p_anim = zalloc(sizeof(struct waterfalls_anim));
    ASSERT(__this->p_anim);
    end_dist = 0x7ffffff;
    //居中计算
    //y方向的计算
    for (int i = 0; i < ARRAY_SIZE(waterfalls_center_y_tab); i++) {
        if (ABS(waterfalls_center_y_tab[i] - __this->icon_center.y) < ABS(end_dist)) {
            end_dist = waterfalls_center_y_tab[i] - __this->icon_center.y;
        }
    }
    //x方向的居中计算
    int end_dist_x = 0 - __this->icon_center.x;
    //x分量与y分量的比
    if (end_dist) {
        __this->p_anim->tanx = 1024 * end_dist_x / end_dist;
    } else {
        __this->p_anim->only_x_en = 1;
        __this->p_anim->tanx  = 1024;
        end_dist = end_dist_x;
    }
    int run_time = 50 + ABS(end_dist) / 2;
    start_dist = 0;
    /* printf("%s %d %d",__func__,run_time,end_dist); */
    /*重新开始配置惯性*/
    ui_anim_init(&__this->p_anim->anim);
    ui_anim_set_var(&__this->p_anim->anim, STAR_LAYOUT);
    ui_anim_set_path_cb(&__this->p_anim->anim, ui_anim_path_ease_out); 	// 过渡效果
    ui_anim_set_exec_cb(&__this->p_anim->anim, waterfalls_anim_exec_cb);		// 运行回调
    ui_anim_set_values(&__this->p_anim->anim, start_dist, end_dist);		// 路径设置
    ui_anim_set_time(&__this->p_anim->anim, run_time);						// 运行时间设置
    ui_anim_start(&__this->p_anim->anim);
}

static void icon_enter_anim_callback(int var, int32_t v)
{
    /*按比例缩放*/
    struct element *elm = ui_core_get_element_by_id(var);
    if (!elm) {
        return;
    }
    if (!(__this->p_anim && __this->p_anim->icon_ratio_tab)) {
        return;
    }
    float *icon_ratio_tab = __this->p_anim->icon_ratio_tab;
    struct element *p;
    int count = 0;
    list_for_each_child_element(p, elm) {
        if (count < 17) {
            /*计算缩放系数，赋值*/
            float icon_ratio = (float)v / 1024 * icon_ratio_tab[count];
            ui_core_set_element_ratio(p, icon_ratio, icon_ratio, 1);
        } else {
            break;
        }
        count ++;
    }
    /*刷新*/
    ui_core_redraw(elm);
}

static void icon_enter_anim(struct element *elm)
{
    waterfalls_anim_stop();
    __this->p_anim = zalloc(sizeof(struct waterfalls_anim));
    ASSERT(__this->p_anim);
    __this->p_anim->icon_ratio_tab = zalloc(sizeof(float) * 17);
    ASSERT(__this->p_anim->icon_ratio_tab);

    /*记录缩放比例*/
    float *icon_ratio_tab = __this->p_anim->icon_ratio_tab;
    int count = 0;
    struct element *p;
    list_for_each_child_element(p, elm) {
        if (count < 17) {
            icon_ratio_tab[count] = p->css.part->ratio.ratio_w;
        } else {
            break;
        }
        count ++;
    }

    /*启动进入动画*/
    int start_dist = 512;				//0.5*1024，放大1024倍，转成整形
    int end_dist = 1024;				//1.0*1024，放大1024倍，转成整型
    int run_time = 300;
    ui_anim_init(&__this->p_anim->anim);
    ui_anim_set_var(&__this->p_anim->anim, STAR_LAYOUT);
    ui_anim_set_path_cb(&__this->p_anim->anim, ui_anim_path_ease_out); 	// 过渡效果
    ui_anim_set_exec_cb(&__this->p_anim->anim, icon_enter_anim_callback);		// 运行回调
    ui_anim_set_values(&__this->p_anim->anim, start_dist, end_dist);		// 路径设置
    ui_anim_set_time(&__this->p_anim->anim, run_time);						// 运行时间设置
    ui_anim_set_ready_cb(&__this->p_anim->anim, waterfalls_anim_ready);
    ui_anim_start(&__this->p_anim->anim);

}

static void icon_onkey_jump_anim(struct element *elm, int dir)
{
    waterfalls_anim_stop();
    int end_dist = 0;
    //y方向的计算
    if (dir ==  KEY_UI_MINUS) {
#if  LOOP_ENABLE
        end_dist = LOOP_DISTANCE  - __this->icon_center.y;
#else
        for (int i = ARRAY_SIZE(waterfalls_center_y_tab) - 1; i >= 0; i--) {
            printf("%s %d %d", __func__, waterfalls_center_y_tab[i], __this->icon_center.y);
            if (waterfalls_center_y_tab[i] > __this->icon_center.y) {
                end_dist = ABS(waterfalls_center_y_tab[i] - __this->icon_center.y);
                break;
            }
        }
#endif
    } else  if (dir == KEY_UI_PLUS) {
#if LOOP_ENABLE
        end_dist = LOOP_DISTANCE - __this->icon_center.y;
        end_dist *= -1;
#else
        for (int i = 0; i < ARRAY_SIZE(waterfalls_center_y_tab); i++) {
            printf("%s %d %d", __func__, waterfalls_center_y_tab[i], __this->icon_center.y);
            if (waterfalls_center_y_tab[i] < __this->icon_center.y) {
                end_dist = -1 * ABS(waterfalls_center_y_tab[i] - __this->icon_center.y);
                break;
            }
        }
#endif
    } else {
        return;
    }
    __this->p_anim = zalloc(sizeof(struct waterfalls_anim));
    ASSERT(__this->p_anim);

    //x方向的居中计算
    int end_dist_x = 0 - __this->icon_center.x;
    //x分量与y分量的比
    if (end_dist) {
        __this->p_anim->tanx = 1024 * end_dist_x / end_dist;
    } else {
        __this->p_anim->only_x_en = 1;
        __this->p_anim->tanx  = 1024;
        end_dist = end_dist_x;
    }
    int run_time = 50 + ABS(end_dist) / 2;
    int start_dist = 0;

    ui_anim_init(&__this->p_anim->anim);
    ui_anim_set_var(&__this->p_anim->anim, STAR_LAYOUT);
    ui_anim_set_path_cb(&__this->p_anim->anim, ui_anim_path_ease_out); 	// 过渡效果
    ui_anim_set_exec_cb(&__this->p_anim->anim, waterfalls_anim_exec_cb);		// 运行回调
    ui_anim_set_values(&__this->p_anim->anim, start_dist, end_dist);		// 路径设置
    ui_anim_set_time(&__this->p_anim->anim, run_time);						// 运行时间设置
    /* ui_anim_set_ready_cb(&__this->p_anim->anim, waterfalls_anim_ready); */
    ui_anim_start(&__this->p_anim->anim);

}

/******************************************************************
                控件事件注册
******************************************************************/
static int LAYOUT_STYLE_STAR_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    switch (event) {
    case ON_CHANGE_INIT:
        /* ui_auto_shut_down_disable(); */
        break;
    case ON_CHANGE_SHOW_POST:
#if ICON_DEBUG
        int rounded_rect_left    = SCREEN_LEFT + ROUNDED_RECTANGLE_R;
        int rounded_rect_right   = SCREEN_RIGHT - ROUNDED_RECTANGLE_R;
        int rounded_rect_top     = SCREEN_TOP + ROUNDED_RECTANGLE_R;
        int rounded_rect_bottom  = SCREEN_BOTTOM - ROUNDED_RECTANGLE_R;
        ui_draw_line(arg, SCREEN_LEFT, SCREEN_TOP, SCREEN_LEFT, SCREEN_BOTTOM, 0x1f);
        ui_draw_line(arg, SCREEN_RIGHT, SCREEN_TOP, SCREEN_RIGHT, SCREEN_BOTTOM, 0x1f);
        ui_draw_line(arg, SCREEN_LEFT, SCREEN_TOP, SCREEN_RIGHT, SCREEN_TOP, 0x1f);
        ui_draw_line(arg, SCREEN_LEFT, SCREEN_BOTTOM, SCREEN_RIGHT, SCREEN_BOTTOM, 0x1f);
        ui_draw_ring(arg, rounded_rect_left, rounded_rect_top, ROUNDED_RECTANGLE_R + 1, ROUNDED_RECTANGLE_R, 180, 270, 0x1f, 100);
        ui_draw_ring(arg, rounded_rect_right, rounded_rect_top, ROUNDED_RECTANGLE_R + 1, ROUNDED_RECTANGLE_R, 270, 360, 0x1f, 100);
        ui_draw_ring(arg, rounded_rect_left, rounded_rect_bottom, ROUNDED_RECTANGLE_R + 1, ROUNDED_RECTANGLE_R, 90, 190, 0x1f, 100);
        ui_draw_ring(arg, rounded_rect_right, rounded_rect_bottom, ROUNDED_RECTANGLE_R + 1, ROUNDED_RECTANGLE_R, 0, 90, 0x1f, 100);
#endif //ICON_DEBUG
        break;
    case ON_CHANGE_FIRST_SHOW:
        __this->ctrl = _ctrl;
        struct layout *layout = (struct layout *)_ctrl;
        icon_init(); //初始化信息
        if (ui_menu_enter_anim_flag_get()) {
            ui_menu_enter_anim_disable();
            icon_postion(layout, 0, 0, 0, 1, 0);
            icon_enter_anim((struct element *)layout);
        } else {
            icon_postion(layout, 0, 0, 0, 1, 1);
        }
        /* key_ui_takeover(1); */
        break;
    case ON_CHANGE_RELEASE:
        waterfalls_anim_stop();
        /* key_ui_takeover(0); */

        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int LAYOUT_STYLE_STAR_onkey(void *_ctr, struct element_key_event *e)
{
    struct element *elm = (struct element *)_ctr;
    printf("%s %d", __func__, e->value);
    switch (e->value) {

    case KEY_UI_MINUS:
        icon_onkey_jump_anim(elm, e->value);
        break;
    case KEY_UI_PLUS:

        icon_onkey_jump_anim(elm, e->value);
        break;
    case KEY_UI_HOME://回表盘
        UI_HIDE_CURR_WINDOW();
        UI_SHOW_WINDOW(ID_WINDOW_DIAL);
        break;
    default:
        break;
    }

    return false;
}

static int LAYOUT_STYLE_STAR_ontouch(void *_ctrl, struct element_touch_event *e)
{
    int ret = false;
#if 1
    /*入场动画不响应触摸*/
    if (__this  && __this->p_anim && __this->p_anim->icon_ratio_tab) {
        return true;
    }
#endif
    struct element *elm = (struct element *)_ctrl;
    switch (e->event) {
    case ELM_EVENT_TOUCH_ENERGY:
        if (__this->down_flag) {
            waterfalls_anim_start(e);
            __this->down_flag = 0;
        }
        break;

    case ELM_EVENT_TOUCH_DOWN:
        waterfalls_anim_stop();
        __this->hold_flag = 0;
        __this->down_flag = 1;
        /* break; */
        return true;
    case ELM_EVENT_TOUCH_HOLD:
        __this->hold_flag = 1;
        break;
    case ELM_EVENT_TOUCH_MOVE:
        if (__this->down_flag) {
            ret = icon_move(_ctrl, e);
        }
        /* break; */
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (e->has_energy) {
            break;
        }
        __this->down_flag = 0;
        if (__this->hold_flag) {
            break;
        }
        if (e->move_flag) {
            // 居中处理
            waterfalls_center_anim_start(e);
            /* get_icon_nearest(); */
            /* anim_button_hander(1); */
            break;
        }
        int touch_index =  icon_touch_up(elm, e);

        //进入满天星子页面
        if (touch_index != -1) {
            int window_id = ui_menu_map_by_sel(touch_index, MENU_SEL_ID_APP_MENU);
            if (window_id > 0) {
                UI_SHOW_WINDOW(window_id);
            }
        }
        /* break; */
        return true;
    case ELM_EVENT_TOUCH_R_MOVE:
    case ELM_EVENT_TOUCH_L_MOVE:
        return true;
    }
    return ret;
}

REGISTER_UI_EVENT_HANDLER(STAR_LAYOUT)
.onchange = LAYOUT_STYLE_STAR_onchange,
 .onkey = LAYOUT_STYLE_STAR_onkey,
  .ontouch = LAYOUT_STYLE_STAR_ontouch,
};


#endif
#endif

