#include "mm32_system.h"
#include "encoder.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"

extern lv_indev_t * indev_encoder;
static lv_obj_t * label;

#if 0
/**
 * Basic example to create a "Hello world" label
 */
void lv_example_get_started_1(void)
{
    /*Change the active screen's background color*/
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x000000), LV_PART_MAIN);

    /*Create a white label, set its text and align it to the center*/
    lv_obj_t * label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "test");
    lv_obj_set_style_text_color(lv_screen_active(), lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 30);
}
#else

static void slider_event_cb(lv_event_t * e)
{
    lv_obj_t * slider = lv_event_get_target(e);

    /*Refresh the text*/
    lv_label_set_text_fmt(label, "%"LV_PRId32, lv_slider_get_value(slider));
    lv_obj_align_to(label, slider, LV_ALIGN_OUT_TOP_MID, 0, -15); /*Align top of the slider*/
}

/**
 * Create a slider and write its value on a label.
 */
lv_obj_t * lv_example_get_started_4(void)
{
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x000000), LV_PART_MAIN);

    /*Create a slider in the center of the display*/
    lv_obj_t * slider = lv_slider_create(lv_screen_active());
    lv_obj_set_width(slider, 200); /*Set the width*/
    lv_obj_center(slider); /*Align to the center of the parent (screen)*/
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL); /*Assign an event function*/

    /*Create a label above the slider*/
    label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "0");
    lv_obj_align_to(label, slider, LV_ALIGN_OUT_TOP_MID, 0, -15); /*Align top of the slider*/
    return slider;
}
#endif



void systick(void)
{
    lv_tick_inc(1);
}

int main()
{
    /* ====  System Clock & SysTick & AppTaskTick Setting  ==== */
    /* When the parameter is NULL, AppTaskTick function used */
    MCUID = SetSystemClock(emSYSTICK_On, systick);

    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();

    //lv_theme_t * th = lv_theme_simple_init(lv_display_get_default());
    //lv_display_set_theme(lv_display_get_default(), th);
    //lv_theme_default_init();

    lv_obj_t * slider = lv_example_get_started_4();

    lv_group_t * g = lv_group_create();
    lv_group_add_obj(g, slider);
    lv_indev_set_group(indev_encoder, g);
    lv_group_set_editing(g, 1);

    /*Create a label under the slider*/
    lv_obj_t * label2 = lv_label_create(lv_screen_active());
    lv_label_set_text(label2, "00");
    lv_obj_align_to(label2, slider, LV_ALIGN_OUT_TOP_MID, 0, 40); /*Align under the slider*/

    uint8_t lv_task_cnt = 0;

    uint16_t sec_cnt = 0;
    uint8_t sec_number = 0;

    while (1)
    {
        // run every 2ms
        if (sysTickFlag)
        {
            sysTickFlag = false;
            encoder_check();

            if (encoder_key_pressed())
            {
                if (sec_number)
                {
                    lv_label_set_text_fmt(label2, "%02d", 0);
                }
                sec_cnt = 0;
                sec_number = 0;
            }

            sec_cnt++;
            if (sec_cnt >= 500)
            {
                sec_cnt = 0;
                sec_number++;
                if (sec_number > 60)
                {
                    sec_number = 0;
                }
                lv_label_set_text_fmt(label2, "%02d", sec_number);
            }

            // run lv_task every 10ms
            lv_task_cnt++;
            if (lv_task_cnt >= 5)
            {
                lv_task_cnt = 0;
                lv_task_handler();
            }
        }
    }

    return 0;
}
