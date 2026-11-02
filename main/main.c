#include "bsp_audio.h"
#include "bsp_battery.h"
#include "bsp_button.h"
#include "bsp_display.h"
#include "bsp_i2c.h"
#include "bsp_pins.h"
#include "dice_model.h"
#include "dice_ui.h"
#include "key_guard.h"
#include "esp_attr.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_random.h"
#include "esp_sleep.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include <stdatomic.h>

#define DICE_RTC_MAGIC 0x44494331u

typedef struct {
    bsp_btn_t button;
    bsp_btn_ev_t event;
    uint64_t at_ms;
} dice_input_t;

static const char *TAG = "dice";
static RTC_DATA_ATTR dice_model_t s_model;
static RTC_DATA_ATTR uint32_t s_rtc_magic;
static QueueHandle_t s_input_queue;
static atomic_bool s_input_ready;
static key_guard_t s_key_guard;
static bool s_drop_until_press;

static uint64_t now_ms(void) {
    return (uint64_t)esp_timer_get_time() / 1000;
}

static uint32_t random_word(void *context) {
    (void)context;
    return esp_random();
}

static void on_key(bsp_btn_t button, bsp_btn_ev_t event, void *user) {
    (void)user;
    if (key_guard_consume(&s_key_guard, event == BSP_BTN_RELEASE, esp_timer_get_time())) return;
    if (!s_input_ready) return;
    if (event == BSP_BTN_PRESS) s_drop_until_press = false;
    if (s_drop_until_press) return;
    const dice_input_t input = {.button = button, .event = event, .at_ms = now_ms()};
    (void)xQueueSend(s_input_queue, &input, 0);
}

static bool dispatch_input(const dice_input_t *input) {
    dice_action_t action;
    if (input->event == BSP_BTN_LONG) {
        if (input->button == BSP_BTN_OK) action = DICE_OK_HOLD;
        else if (input->button == BSP_BTN_DOWN) action = DICE_DOWN_HOLD;
        else return false;
    } else if (input->event == BSP_BTN_CLICK || input->event == BSP_BTN_DOUBLE) {
        action = input->button == BSP_BTN_UP ? DICE_UP
               : input->button == BSP_BTN_DOWN ? DICE_DOWN : DICE_OK;
    } else return false;
    bool changed = dice_model_action(&s_model, action, input->at_ms, random_word, NULL);
    if (input->event == BSP_BTN_DOUBLE && input->button != BSP_BTN_OK) {
        changed |= dice_model_action(&s_model, action, input->at_ms, random_word, NULL);
    }
    return changed;
}

static void warn_step(const char *step, esp_err_t error) {
    if (error != ESP_OK) ESP_LOGW(TAG, "%s: %s", step, esp_err_to_name(error));
}

static void enter_sleep(void) {
    if (bsp_button_read_mv() < BSP_BTN_MV_RELEASED_MIN) return;
    if (esp_deep_sleep_enable_gpio_wakeup(1ULL << BSP_BTN_GPIO, ESP_GPIO_WAKEUP_GPIO_LOW) != ESP_OK) {
        ESP_LOGE(TAG, "Cannot arm key wake; staying awake");
        return;
    }
    s_input_ready = false;
    int level = 0;
    if (bsp_button_prepare_deep_sleep(&level) != ESP_OK || level != 1) esp_restart();
    warn_step("battery suspend", bsp_battery_sleep());
    warn_step("audio suspend", bsp_audio_sleep());
    warn_step("audio pins", bsp_audio_prepare_deep_sleep());
    warn_step("I2C pins", bsp_i2c_prepare_deep_sleep());
    if (!bsp_lvgl_lock(1000)) esp_restart();
    warn_step("display suspend", bsp_display_prepare_deep_sleep());
    s_rtc_magic = DICE_RTC_MAGIC;
    ESP_LOGI(TAG, "Idle deep sleep; any key wakes; session retained in RTC");
    esp_deep_sleep_start();
    esp_restart();
}

void app_main(void) {
    const bool key_wake = esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_GPIO;
    if (!key_wake || s_rtc_magic != DICE_RTC_MAGIC) dice_model_init(&s_model);
    if (key_wake) {
        key_guard_arm(&s_key_guard, esp_timer_get_time(), 10LL * 1000 * 1000);
        s_drop_until_press = true;
    }
    ESP_LOGI(TAG, "passport-pocket-dice v0.2; session %s", key_wake ? "resumed" : "new");
    warn_step("I2C initialization", bsp_i2c_init());
    if (bsp_display_init() != ESP_OK || !bsp_lvgl_init()) {
        ESP_LOGE(TAG, "Display initialization failed");
        return;
    }
    bsp_display_backlight(0);
    if (bsp_audio_init() == ESP_OK) warn_step("idle audio suspend", bsp_audio_sleep());
    const bool battery_ok = bsp_battery_init() == ESP_OK;
    s_input_queue = xQueueCreate(16, sizeof(dice_input_t));
    if (!s_input_queue) {
        ESP_LOGE(TAG, "Input queue allocation failed");
        return;
    }
    const bool buttons_ok = bsp_button_init(on_key, NULL) == ESP_OK;
    if (s_key_guard.armed) {
        key_guard_sync_released(&s_key_guard, bsp_button_read_mv() >= BSP_BTN_MV_RELEASED_MIN);
    }
    int battery = battery_ok ? bsp_battery_soc() : -1;
    if (!bsp_lvgl_lock(1000)) {
        ESP_LOGE(TAG, "Initial UI lock failed");
        return;
    }
    if (!dice_ui_font_check()) {
        ESP_LOGE(TAG, "Chinese font coverage failed");
        bsp_lvgl_unlock();
        return;
    }
    dice_ui_create();
    dice_ui_render(&s_model, battery, now_ms(), buttons_ok);
    bsp_lvgl_unlock();
    s_input_ready = true;
    uint64_t last_activity = now_ms();
    uint64_t battery_polled = 0;
    uint64_t rendered_frame = UINT64_MAX;
    uint8_t backlight = 0;
    bool dirty = true;
    bool wake_only = false;
    ESP_LOGI(TAG, "Font coverage PASS; free=%u largest=%u", (unsigned)esp_get_free_heap_size(),
             (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
    for (;;) {
        dice_input_t input;
        if (xQueueReceive(s_input_queue, &input, pdMS_TO_TICKS(40)) == pdTRUE) {
            if (input.event == BSP_BTN_PRESS) wake_only = backlight == 0;
            last_activity = now_ms();
            if (!wake_only) dirty |= dispatch_input(&input);
        }
        const uint64_t current = now_ms();
        if (s_model.rolling) last_activity = current;
        dirty |= dice_model_tick(&s_model, current);
        if (battery_ok && (battery_polled == 0 || current - battery_polled >= 30000)) {
            battery = bsp_battery_soc();
            battery_polled = current;
            dirty = true;
        }
        const uint64_t frame = s_model.rolling ? (current - s_model.roll_started_ms) / 80 : 0;
        if (frame != rendered_frame) dirty = true;
        if (dirty && bsp_lvgl_lock(200)) {
            dice_ui_render(&s_model, battery, current, buttons_ok);
            bsp_lvgl_unlock();
            dirty = false;
            rendered_frame = frame;
        }
        const uint8_t brightness = dice_brightness(current - last_activity);
        if (brightness != backlight) {
            bsp_display_backlight(brightness);
            backlight = brightness;
        }
        if (buttons_ok && current - last_activity >= DICE_SLEEP_MS) {
            enter_sleep();
            last_activity = now_ms();
        }
    }
}
