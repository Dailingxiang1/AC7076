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
#define LOG_TAG     		"[UI_MENU_LIST]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_menu_list.data.bss")
#pragma data_seg(".ui_action_menu_list.data")
#pragma const_seg(".ui_action_menu_list.text.const")
#pragma code_seg(".ui_action_menu_list.text")
#endif

#ifdef CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE
#if TCFG_UI_MENU_LIST_ENABLE

#define STYLE_NAME  JL
REGISTER_UI_STYLE(STYLE_NAME)

#define MENU_GRID_VLIST 				0X61400

#define GRID_ROTATE_MAP_EN				1
#define GRID_ROTATE_MAP_TYPE_ARC		0
#define GRID_ROTATE_MAP_CENTER_X		420
#define GRID_ROTATE_MAP_CENTER_Y		192
#define GRID_ROTATE_MAP_HEIGHT			386
#define GRID_ROTATE_MAP_WIDTH			320

#if GRID_ROTATE_MAP_TYPE_ARC
#define GRID_ROTATE_MAP_RADIO			400
#else
#define GRID_ROTATE_MAP_RADIO			90
#endif
struct menu_list_anim {
    ui_anim_t grid_enter_anim[5];
    u8 redraw_count;
    u8 count;
};
static int grid_move_steps = 0;
static struct menu_list_anim *p_menu_anim = NULL;
void menu_list_rec_clr()
{
    grid_move_steps = 0;
}
static int grid_rotate_map(struct ui_grid *grid)
{
#if (GRID_ROTATE_MAP_TYPE_ARC)
    //圆弧
    int lcd_h = GRID_ROTATE_MAP_HEIGHT;
    int cx = GRID_ROTATE_MAP_CENTER_X;							//圆心
    int cy = lcd_h / 2;												//圆心
    int cr = GRID_ROTATE_MAP_RADIO;											//半径

    struct rect item_rect;
    struct element *item_elm;
    struct rect grid_rect;
    int index = 0;
    ui_core_get_element_abs_rect((struct element *)grid, &grid_rect);
    list_for_each_child_element(item_elm, (struct element *)grid) {	//遍历列表所有项
        ui_core_get_element_abs_rect(item_elm, &item_rect);			//获取项的rect
        int iy = (item_rect.top + item_rect.height / 2);			//计算项的中心y值
        int iy_top = item_rect.top;
        int iy_btm = item_rect.top + item_rect.height;
        if ((iy_btm <= 0) || (iy_top >= lcd_h)) {								//不在屏幕内，则
            item_elm->css.left = grid_rect.width;								//移出可视范围（这里不能用隐藏）
        } else {														//否则
            double xx = (cr * cr - (iy - cy) * (iy - cy));			//计算距离屏幕竖直中线的距离
            int ix = cr - root_float(xx) + (cx - cr);					//求解得到水平坐标
            int cssx = ix;				//坐标换算
            item_elm->css.left = cssx;								//赋值给left
            /* float angle = (float)abs(iy-cy)/500/2; */
            /* float ratio = cos_float(angle);   */
            /* ui_core_set_element_ratio(item_elm,ratio,ratio,1); */
            //printf("%s index:%d iy:%d xx:%f ix:%d cssx:%d cssy:%d",__func__,index,iy,xx,ix,cssx,item_elm->css.top);
        }
        index++;
    }
#else
    //圆角矩形左半边
    int lcd_h = GRID_ROTATE_MAP_HEIGHT;
    int lcd_w = GRID_ROTATE_MAP_WIDTH;
    int item_height_half = 45;
    int artc_r = 90 ;
    struct rect item_rect;
    struct element *item_elm;
    struct rect grid_rect;
    int index = 0;
    ui_core_get_element_abs_rect((struct element *)grid, &grid_rect);
    list_for_each_child_element(item_elm, (struct element *)grid) {	//遍历列表所有项
        ui_core_get_element_abs_rect(item_elm, &item_rect);			//获取项的rect
        int iy = (item_rect.top + item_rect.height / 2);			//计算项的中心y值
        int iy_top = item_rect.top;
        int iy_btm = item_rect.top + item_rect.height;
        int ix = 0;
        int artc_y = 0;
        if ((iy_btm <= 0) || (iy_top >= lcd_h)) {								//不在屏幕内，则
            item_elm->css.left = grid_rect.width;								//移出可视范围（这里不能用隐藏）
        } else {														//否则
            if (iy <= artc_r) {
                artc_y = artc_r - iy;
            } else if (iy >= lcd_h - artc_r) {
                artc_y =  iy - (lcd_h - artc_r) ;
            } else {
                artc_y = 0;
            }
            ix = artc_r + item_height_half - root_float((float)((artc_r + item_height_half) * (artc_r + item_height_half) -  artc_y * artc_y));
            /* printf("%s iy:%d ix:%d atrcy:%d artc_r:%d comd:%f \n", __func__, iy, ix, artc_y, artc_r, root_float((float)(artc_r * artc_r -  artc_y * artc_y))); */
            item_elm->css.left = ix;								//赋值给left
        }
        /* printf("%s %d \n", __func__, item_elm->css.left); */
        index++;
    }
#endif
    return 0;
}

static int menu_grid_vlist_child_onchange(void *ctrl, enum element_change_event event, void *arg)
{
    struct element *elm = (struct element *)ctrl;
    if (event == ON_CHANGE_UPDATE_ITEM) {
        int index = (int)arg;
        /* printf("%s id%x index%d type:%d \n", __func__, elm->id, index, ui_id2type(elm->id)); */
        if (ui_id2type(elm->id) == CTRL_TYPE_PIC) {
            ui_pic_set_image_index((struct ui_pic *)elm, index);
        } else if (ui_id2type(elm->id) == CTRL_TYPE_TEXT) {
            ui_text_set_index((struct ui_text *)elm, index);
        }

    }
    return false;
}
static void menu_list_enter_anim_cb(int var, int v)
{

    struct element *elm = ui_core_get_element_by_id(var);
    if (!elm) {
        return;
    }
    elm->css.left = v;
    p_menu_anim->redraw_count++;
    if (p_menu_anim->redraw_count == 5) {
        ui_core_redraw(elm->parent);
        p_menu_anim->redraw_count = 0;
    }
    /* printf("%s v:%d count:%d", __func__, v, p_menu_anim->redraw_count); */
}
static void menu_list_anim_stop()
{
    struct ui_grid *grid = (struct ui_grid *)ui_core_get_element_by_id(MENU_GRID_VLIST);
    if (!grid) {
        return;
    }
    if (!p_menu_anim) {
        return;
    }
    for (int i = 0; i < 5; i++) {
        ui_anim_del(grid->item[i].elm.id, NULL);
    }
    free(p_menu_anim);
    p_menu_anim = NULL;
}
static void menu_list_anim_ready(struct _ui_anim_t *p)
{
    /* int var = p->var; */
    p_menu_anim->count --;
    if (!p_menu_anim->count) {
        menu_list_anim_stop();
    }
    /* printf("%s %d", __func__, __LINE__); */
}
static void menu_list_enter_anim(struct ui_grid *grid)
{
    menu_list_anim_stop();
    for (int i = 0; i < 5; i++) {
        grid->item[i].elm.css.left += grid->item[i].elm.css.width + i * 80;
    }
    /*进入动画*/
    p_menu_anim = zalloc(sizeof(struct  menu_list_anim));
    for (int i = 0; i < 5; i++) {
        int start_pos  = grid->item[i].elm.css.left;
        int end_pos =  grid->item[i].elm.css.left - grid->item[i].elm.css.width - i * 80;

        ui_anim_init(&p_menu_anim->grid_enter_anim[i]);
        ui_anim_set_var(&p_menu_anim->grid_enter_anim[i], grid->item[i].elm.id);
        ui_anim_set_path_cb(&p_menu_anim->grid_enter_anim[i], ui_anim_path_ease_out);
        ui_anim_set_exec_cb(&p_menu_anim->grid_enter_anim[i], menu_list_enter_anim_cb);
        ui_anim_set_values(&p_menu_anim->grid_enter_anim[i], start_pos, end_pos);
        ui_anim_set_time(&p_menu_anim->grid_enter_anim[i], 300);
        ui_anim_set_ready_cb(&p_menu_anim->grid_enter_anim[i], menu_list_anim_ready);
        ui_anim_start(&p_menu_anim->grid_enter_anim[i]);
        p_menu_anim->count ++;
    }
}


static int menu_grid_vlist_onchange(void *ctrl, enum element_change_event event, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctrl;
    struct element *elm = (struct element *)ctrl;
    struct draw_context *dc = NULL;
    switch (event) {
    case ON_CHANGE_INIT_PROBE:
        break;
    case ON_CHANGE_INIT:
        ui_grid_set_slide_direction(grid, 2);
        int row = 32;
        int col = 1;
        ui_set_default_handler(elm, NULL, NULL, menu_grid_vlist_child_onchange);
        ui_grid_init_dynamic(grid, &row, &col);
        ui_grid_set_base_dynamic(grid, 0, grid_move_steps + 1);
        ui_grid_energy_auto_center(grid, AUTO_CENTER_MODE1);
        ui_grid_set_energy_target_line(grid, 140);
        grid_rotate_map(grid);

        break;
    case ON_CHANGE_SHOW_PROBE:
        if (!p_menu_anim) {
            grid_rotate_map((struct ui_grid *)elm);
        }
        break;
    case ON_CHANGE_SHOW:
        break;
    case ON_CHANGE_SHOW_POST:
        break;
    case ON_CHANGE_FIRST_SHOW:
        if (ui_menu_enter_anim_flag_get()) {
            ui_menu_enter_anim_disable();
            menu_list_enter_anim(grid);
        }
        break;
    case ON_CHANGE_RELEASE:
        /*释放*/
        if (grid->item) {
            struct rect item_r;
            ui_core_get_element_abs_rect((struct element *)&grid->item[0], &item_r);
            int item_height = item_r.height + grid->y_interval;
            int item_num = grid->dynamic->min_row_index;
            if (item_r.top + item_r.height / 2 < 0) {
                item_num ++;
            }
            grid_move_steps = -1 * item_num  * item_height;
        }
        menu_list_anim_stop();
        break;
    default:
        return false;
    }
    return false;
}
static int menu_grid_vlist_ontouch(void *ctrl, struct element_touch_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctrl;
    struct element *elm = (struct element *)ctrl;
#if 0
    /*按下停止入场动画*/
    if (e->event == ELM_EVENT_TOUCH_DOWN) {
        menu_list_anim_stop();
    }
#else
    /*入场动画不响应触摸*/
    if (p_menu_anim) {
        return true;
    }
#endif
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:

        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        if (!e->move_flag) {
            int touch_index = ui_grid_touch_item((struct ui_grid *)elm);
            if (touch_index != -1) {
                touch_index  = ui_grid_cur_item_dynamic((struct ui_grid *)elm);
                int window_id = ui_menu_map_by_sel(touch_index, MENU_SEL_ID_APP_MENU);
                if (window_id > 0) {
                    /* UI_SHOW_WINDOW(window_id); */
                    extern void menu_enter_app_anim(u32 app_id, int touch_x, int touch_y);
                    menu_enter_app_anim(window_id, e->pos.x, e->pos.y);
                }


            }
        }
        break;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(MENU_GRID_VLIST)
.onchange =  menu_grid_vlist_onchange,
 .onkey = NULL,
  .ontouch =  menu_grid_vlist_ontouch,
};

#endif// TCFG_UI_DRAW_DEMO
#endif// CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE
