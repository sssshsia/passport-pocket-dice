#pragma once

#include <stdbool.h>
#include <stdint.h>

#define DICE_TYPE_COUNT 8
#define DICE_MAX_COUNT 6
#define DICE_HISTORY_SIZE 12
#define DICE_ROLL_MS 720
#define DICE_SLEEP_MS 300000

typedef enum {
    DICE_HOME,
    DICE_COUNT,
    DICE_HISTORY,
} dice_page_t;

typedef enum {
    DICE_UP,
    DICE_DOWN,
    DICE_OK,
    DICE_OK_HOLD,
    DICE_DOWN_HOLD,
} dice_action_t;

typedef uint32_t (*dice_random_fn)(void *context);

typedef struct {
    uint8_t type;
    uint8_t count;
    uint8_t values[DICE_MAX_COUNT];
    uint16_t total;
} dice_result_t;

typedef struct {
    dice_page_t page;
    uint8_t type;
    uint8_t count;
    uint8_t history_count;
    uint8_t history_cursor;
    bool rolling;
    bool has_result;
    bool random_error;
    uint64_t roll_started_ms;
    dice_result_t result;
    dice_result_t history[DICE_HISTORY_SIZE];
} dice_model_t;

extern const uint8_t dice_sides[DICE_TYPE_COUNT];

void dice_model_init(dice_model_t *model);
bool dice_uniform(uint8_t sides, dice_random_fn random, void *context, uint8_t *value);
bool dice_model_action(dice_model_t *model, dice_action_t action, uint64_t now_ms,
                       dice_random_fn random, void *context);
bool dice_model_tick(dice_model_t *model, uint64_t now_ms);
uint8_t dice_brightness(uint64_t idle_ms);
