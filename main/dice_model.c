#include "dice_model.h"
#include <string.h>

const uint8_t dice_sides[DICE_TYPE_COUNT] = {2, 4, 6, 8, 10, 12, 20, 100, 200};

void dice_model_init(dice_model_t *model) {
    memset(model, 0, sizeof(*model));
    model->type = 2;
    model->count = 1;
}

bool dice_uniform(uint8_t sides, dice_random_fn random, void *context, uint8_t *value) {
    if (sides < 2 || !random || !value) return false;
    const uint32_t threshold = (uint32_t)(0u - (uint32_t)sides) % sides;
    for (unsigned attempt = 0; attempt < 32; ++attempt) {
        const uint32_t sample = random(context);
        if (sample >= threshold) {
            *value = (uint8_t)(sample % sides + 1);
            return true;
        }
    }
    return false;
}

bool dice_model_action(dice_model_t *model, dice_action_t action, uint64_t now_ms,
                       dice_random_fn random, void *context) {
    if (model->rolling) return false;
    if (model->page == DICE_HISTORY) {
        if (action == DICE_OK || action == DICE_OK_HOLD) model->page = DICE_HOME;
        else if (action == DICE_UP && model->history_cursor > 0) --model->history_cursor;
        else if (action == DICE_DOWN && model->history_cursor + 1 < model->history_count) {
            ++model->history_cursor;
        } else return false;
        return true;
    }
    if (model->page == DICE_COUNT) {
        if (action == DICE_OK || action == DICE_OK_HOLD) model->page = DICE_HOME;
        else if (action == DICE_UP && model->type != 0 && model->count < DICE_MAX_COUNT) {
            ++model->count;
            model->has_result = false;
        } else if (action == DICE_DOWN && model->type != 0 && model->count > 1) {
            --model->count;
            model->has_result = false;
        } else return false;
        return true;
    }
    if (action == DICE_OK_HOLD) {
        model->page = DICE_COUNT;
    } else if (action == DICE_DOWN_HOLD) {
        model->page = DICE_HISTORY;
        model->history_cursor = 0;
    } else if (action == DICE_UP || action == DICE_DOWN) {
        model->type = (uint8_t)((model->type + (action == DICE_UP ? DICE_TYPE_COUNT - 1 : 1))
                               % DICE_TYPE_COUNT);
        if (model->type == 0) model->count = 1;
        model->has_result = false;
        model->random_error = false;
    } else if (action == DICE_OK) {
        dice_result_t next = {.type = model->type, .count = model->count};
        for (unsigned index = 0; index < next.count; ++index) {
            if (!dice_uniform(dice_sides[next.type], random, context, &next.values[index])) {
                model->random_error = true;
                return true;
            }
            next.total += next.values[index];
        }
        model->result = next;
        model->random_error = false;
        model->has_result = false;
        model->rolling = true;
        model->roll_started_ms = now_ms;
    } else return false;
    return true;
}

bool dice_model_tick(dice_model_t *model, uint64_t now_ms) {
    if (!model->rolling || now_ms - model->roll_started_ms < DICE_ROLL_MS) return false;
    model->rolling = false;
    model->has_result = true;
    const unsigned retained = model->history_count < DICE_HISTORY_SIZE
                            ? model->history_count++ : DICE_HISTORY_SIZE - 1;
    memmove(&model->history[1], &model->history[0], retained * sizeof(model->history[0]));
    model->history[0] = model->result;
    return true;
}

uint8_t dice_brightness(uint64_t idle_ms) {
    if (idle_ms >= 90000) return 0;
    return idle_ms >= 30000 ? 10 : 70;
}
