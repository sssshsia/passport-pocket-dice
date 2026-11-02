#include "dice_model.h"
#include "dice_ui.h"
#include "lvgl.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static uint16_t s_frame[240 * 320];
static uint16_t s_draw_buffer[240 * 20];

static void flush(lv_display_t *display, const lv_area_t *area, uint8_t *pixels) {
    const uint16_t *source = (const uint16_t *)pixels;
    for (int row = area->y1; row <= area->y2; ++row) {
        for (int column = area->x1; column <= area->x2; ++column) {
            assert(row >= 0 && row < 320 && column >= 0 && column < 240);
            s_frame[row * 240 + column] = *source++;
        }
    }
    lv_display_flush_ready(display);
}

static void check_labels(lv_obj_t *object) {
    if (lv_obj_has_flag(object, LV_OBJ_FLAG_HIDDEN)) return;
    if (lv_obj_check_type(object, &lv_label_class)) {
        lv_point_t size;
        lv_text_get_size(&size, lv_label_get_text(object),
                         lv_obj_get_style_text_font(object, LV_PART_MAIN),
                         lv_obj_get_style_text_letter_space(object, LV_PART_MAIN),
                         lv_obj_get_style_text_line_space(object, LV_PART_MAIN),
                         lv_obj_get_width(object), LV_TEXT_FLAG_NONE);
        lv_area_t bounds;
        lv_obj_get_coords(object, &bounds);
        assert(bounds.x1 >= 0 && bounds.x2 < 240);
        assert(bounds.y1 >= 0 && bounds.y1 + size.y <= 320);
    }
    for (unsigned index = 0; index < lv_obj_get_child_count(object); ++index) {
        check_labels(lv_obj_get_child(object, index));
    }
}

static void render(dice_model_t *model, const char *path, uint64_t time_ms, bool buttons_ok) {
    dice_ui_render(model, 76, time_ms, buttons_ok);
    lv_obj_update_layout(lv_screen_active());
    lv_refr_now(NULL);
    check_labels(lv_screen_active());
    if (!path) return;
    FILE *output = fopen(path, "wb");
    assert(output);
    fprintf(output, "P6\n240 320\n255\n");
    for (unsigned index = 0; index < 240 * 320; ++index) {
        const uint16_t pixel = s_frame[index];
        const uint8_t rgb[3] = {
            (uint8_t)(((pixel >> 11) & 31) * 255 / 31),
            (uint8_t)(((pixel >> 5) & 63) * 255 / 63),
            (uint8_t)((pixel & 31) * 255 / 31)
        };
        assert(fwrite(rgb, 1, sizeof(rgb), output) == sizeof(rgb));
    }
    fclose(output);
}

static uint32_t next_random(void *context) {
    uint32_t *value = context;
    return (*value)++;
}

int main(void) {
    lv_init();
    lv_display_t *display = lv_display_create(240, 320);
    assert(display);
    lv_display_set_color_format(display, LV_COLOR_FORMAT_RGB565);
    lv_display_set_buffers(display, s_draw_buffer, NULL, sizeof(s_draw_buffer),
                           LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(display, flush);
    assert(dice_ui_font_check());
    dice_ui_create();
    dice_model_t model;
    dice_model_init(&model);
    render(&model, "dice-ready.ppm", 0, true);
    model.page = DICE_HISTORY;
    render(&model, "dice-empty.ppm", 0, true);
    model.page = DICE_HOME;
    model.count = 6;
    uint32_t sample = 600;
    dice_model_action(&model, DICE_OK, 0, next_random, &sample);
    render(&model, "dice-rolling.ppm", 320, true);
    dice_model_tick(&model, 720);
    render(&model, "dice-result.ppm", 720, true);
    model.page = DICE_COUNT;
    render(&model, "dice-count.ppm", 720, true);
    model.page = DICE_HISTORY;
    render(&model, "dice-history.ppm", 720, true);
    for (unsigned iteration = 0; iteration < 300; ++iteration) {
        model.page = DICE_HOME;
        model.type = iteration % DICE_TYPE_COUNT;
        model.count = model.type == 0 ? 1 : iteration % DICE_MAX_COUNT + 1;
        dice_model_action(&model, DICE_OK, 1000, next_random, &sample);
        dice_model_tick(&model, 1720);
        render(&model, NULL, 1720, true);
        model.page = DICE_COUNT;
        render(&model, NULL, 1720, true);
        model.page = DICE_HISTORY;
        render(&model, NULL, 1720, true);
        assert(lv_mem_test() == LV_RESULT_OK);
    }
    model.page = DICE_HOME;
    model.type = 0;
    model.count = 1;
    dice_model_action(&model, DICE_OK, 0, next_random, &sample);
    dice_model_tick(&model, 720);
    render(&model, "dice-coin.ppm", 720, true);
    model.random_error = true;
    render(&model, "dice-error.ppm", 720, false);
    dice_ui_render(&model, -1, 720, true);
    lv_refr_now(NULL);
    lv_mem_monitor_t memory;
    lv_mem_monitor(&memory);
    printf("Dice UI: PASS; 900 page transitions; pool=%u free=%u max_used=%u\n",
           (unsigned)memory.total_size, (unsigned)memory.free_size, (unsigned)memory.max_used);
    lv_deinit();
    return 0;
}
