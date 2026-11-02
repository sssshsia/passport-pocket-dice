#include "dice_ui.h"
#include "dice_text.h"
#include "lvgl.h"
#include <stdio.h>

LV_FONT_DECLARE(dice_font_16);
extern const uint32_t dice_font_codepoints[];
extern const unsigned dice_font_codepoint_count;

#define COLOR_BG 0x101A28
#define COLOR_PANEL 0x1D2C40
#define COLOR_INK 0xF5F1E7
#define COLOR_MUTED 0xA5B6CC
#define COLOR_ACCENT 0xF4BD63

static lv_obj_t *s_screen;
static lv_obj_t *s_body;
static lv_obj_t *s_battery;
static lv_obj_t *s_caption;
static lv_obj_t *s_result;
static lv_obj_t *s_note;
static lv_obj_t *s_hint;
static lv_obj_t *s_tiles[DICE_MAX_COUNT];
static lv_obj_t *s_faces[DICE_MAX_COUNT];
static lv_obj_t *s_pips[DICE_MAX_COUNT][7];
static int s_page = -1;

static lv_obj_t *panel(lv_obj_t *parent, int left, int top, int width, int height) {
    lv_obj_t *object = lv_obj_create(parent);
    lv_obj_remove_style_all(object);
    lv_obj_set_pos(object, left, top);
    lv_obj_set_size(object, width, height);
    lv_obj_set_style_bg_color(object, lv_color_hex(COLOR_PANEL), 0);
    lv_obj_set_style_bg_opa(object, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(object, 12, 0);
    lv_obj_remove_flag(object, LV_OBJ_FLAG_SCROLLABLE);
    return object;
}

static lv_obj_t *label(lv_obj_t *parent, int left, int top, int width,
                       const lv_font_t *font, uint32_t color, const char *text) {
    lv_obj_t *object = lv_label_create(parent);
    lv_obj_set_pos(object, left, top);
    lv_obj_set_width(object, width);
    lv_obj_set_style_text_font(object, font, 0);
    lv_obj_set_style_text_color(object, lv_color_hex(color), 0);
    lv_obj_set_style_text_align(object, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(object, text);
    return object;
}

bool dice_ui_font_check(void) {
    for (unsigned index = 0; index < dice_font_codepoint_count; ++index) {
        lv_font_glyph_dsc_t descriptor;
        if (!lv_font_get_glyph_dsc(&dice_font_16, &descriptor, dice_font_codepoints[index], 0)
            || descriptor.is_placeholder) return false;
    }
    return true;
}

void dice_ui_create(void) {
    s_screen = lv_obj_create(NULL);
    lv_obj_remove_style_all(s_screen);
    lv_obj_set_style_bg_color(s_screen, lv_color_hex(COLOR_BG), 0);
    lv_obj_set_style_bg_opa(s_screen, LV_OPA_COVER, 0);
    lv_obj_remove_flag(s_screen, LV_OBJ_FLAG_SCROLLABLE);
    label(s_screen, 18, 16, 110, &dice_font_16, COLOR_ACCENT, TXT_TITLE);
    s_battery = label(s_screen, 162, 18, 58, &lv_font_montserrat_14, COLOR_MUTED, "--%");
    s_body = panel(s_screen, 12, 48, 216, 208);
    lv_obj_set_style_bg_opa(s_body, LV_OPA_TRANSP, 0);
    s_hint = label(s_screen, 12, 268, 216, &dice_font_16, COLOR_MUTED, "");
    lv_obj_set_style_text_line_space(s_hint, 4, 0);
    s_page = -1;
    lv_screen_load(s_screen);
}

static void build_body(dice_page_t page) {
    lv_obj_clean(s_body);
    s_caption = label(s_body, 0, 0, 216, &dice_font_16, COLOR_ACCENT, "");
    s_note = label(s_body, 0, 30, 216, &dice_font_16, COLOR_MUTED, "");
    s_result = label(s_body, 0, 54, 216, &lv_font_montserrat_48, COLOR_INK, "");
    for (unsigned index = 0; index < DICE_MAX_COUNT; ++index) {
        s_tiles[index] = panel(s_body, 3 + (int)(index % 3) * 72,
                              114 + (int)(index / 3) * 48, 66, 42);
        s_faces[index] = label(s_tiles[index], 0, 10, 66,
                              &lv_font_montserrat_20, COLOR_INK, "");
        for (unsigned pip = 0; pip < 7; ++pip) {
            static const int positions[7][2] = {
                {19, 8}, {41, 8}, {19, 18}, {30, 18}, {41, 18}, {19, 28}, {41, 28}
            };
            s_pips[index][pip] = panel(s_tiles[index], positions[pip][0], positions[pip][1], 6, 6);
            lv_obj_set_style_radius(s_pips[index][pip], LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_bg_color(s_pips[index][pip], lv_color_hex(COLOR_ACCENT), 0);
        }
    }
    if (page == DICE_COUNT) lv_obj_set_y(s_result, 66);
    s_page = page;
}

static void set_visible(lv_obj_t *object, bool visible) {
    if (visible) lv_obj_remove_flag(object, LV_OBJ_FLAG_HIDDEN);
    else lv_obj_add_flag(object, LV_OBJ_FLAG_HIDDEN);
}

static void show_faces(uint8_t type, uint8_t count, const uint8_t *values, bool ready) {
    static const uint8_t pip_masks[] = {0, 8, 65, 73, 99, 107, 119};
    for (unsigned index = 0; index < DICE_MAX_COUNT; ++index) {
        set_visible(s_tiles[index], index < count);
        const unsigned value = ready && values ? values[index] : 0;
        const bool pips = type == 2 && value > 0;
        set_visible(s_faces[index], !pips);
        if (type == 0 && ready) {
            lv_obj_set_style_text_font(s_faces[index], &dice_font_16, 0);
            lv_label_set_text(s_faces[index], value == 1 ? TXT_HEADS : TXT_TAILS);
        } else {
            lv_obj_set_style_text_font(s_faces[index], &lv_font_montserrat_20, 0);
            if (ready) lv_label_set_text_fmt(s_faces[index], "%u", value);
            else lv_label_set_text(s_faces[index], "?");
        }
        for (unsigned pip = 0; pip < 7; ++pip) {
            set_visible(s_pips[index][pip],
                        pips && value <= 6 && (pip_masks[value] & (1u << pip)));
        }
    }
}

static void show_result(const dice_result_t *result) {
    if (result->type == 0) {
        lv_obj_set_style_text_font(s_result, &dice_font_16, 0);
        lv_label_set_text(s_result, result->values[0] == 1 ? TXT_HEADS : TXT_TAILS);
    } else {
        lv_obj_set_style_text_font(s_result, &lv_font_montserrat_48, 0);
        lv_label_set_text_fmt(s_result, "%u", result->total);
    }
    show_faces(result->type, result->count, result->values, true);
}

void dice_ui_render(const dice_model_t *model, int battery, uint64_t now_ms, bool buttons_ok) {
    if (s_page != (int)model->page) build_body(model->page);
    if (battery < 0) lv_label_set_text(s_battery, "--%");
    else lv_label_set_text_fmt(s_battery, "%d%%", battery);
    lv_obj_set_style_text_font(s_result, &lv_font_montserrat_48, 0);
    if (model->page == DICE_HOME) {
        if (model->type == 0) lv_label_set_text(s_caption, TXT_COIN);
        else lv_label_set_text_fmt(s_caption, "%u x D%u", model->count, dice_sides[model->type]);
        lv_label_set_text(s_note, model->random_error ? TXT_ERROR
                         : model->rolling ? TXT_ROLLING : model->has_result ? TXT_TOTAL : TXT_READY);
        lv_label_set_text(s_hint, TXT_HOME_HINT);
        if (model->rolling) {
            uint8_t frames[DICE_MAX_COUNT];
            const unsigned frame = (unsigned)((now_ms - model->roll_started_ms) / 80);
            for (unsigned index = 0; index < model->count; ++index) {
                frames[index] = (uint8_t)((frame + index * 3) % dice_sides[model->type] + 1);
            }
            lv_label_set_text(s_result, "...");
            show_faces(model->type, model->count, frames, true);
        } else if (model->has_result) show_result(&model->result);
        else {
            lv_label_set_text(s_result, "--");
            show_faces(model->type, model->count, NULL, false);
        }
    } else if (model->page == DICE_COUNT) {
        lv_label_set_text(s_caption, TXT_COUNT);
        lv_label_set_text(s_note, model->type == 0 ? TXT_COIN_NOTE : TXT_COUNT_NOTE);
        lv_label_set_text_fmt(s_result, "%u", model->count);
        lv_label_set_text(s_hint, TXT_COUNT_HINT);
        show_faces(model->type, 0, NULL, false);
    } else {
        lv_label_set_text(s_hint, TXT_HISTORY_HINT);
        if (model->history_count == 0) {
            lv_label_set_text(s_caption, TXT_HISTORY);
            lv_label_set_text(s_note, TXT_EMPTY);
            lv_obj_set_style_text_font(s_result, &dice_font_16, 0);
            lv_label_set_text(s_result, TXT_EMPTY_NOTE);
            show_faces(0, 0, NULL, false);
        } else {
            const dice_result_t *record = &model->history[model->history_cursor];
            lv_label_set_text_fmt(s_caption, TXT_HISTORY "  %u/%u",
                                  model->history_cursor + 1, model->history_count);
            if (record->type == 0) lv_label_set_text(s_note, TXT_COIN);
            else lv_label_set_text_fmt(s_note, "%u x D%u", record->count, dice_sides[record->type]);
            show_result(record);
        }
    }
    if (!buttons_ok) lv_label_set_text(s_hint, TXT_BUTTON_ERROR);
}
