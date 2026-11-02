#include "dice_model.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    uint32_t sample;
    unsigned calls;
} random_fixture_t;

static uint32_t constant_random(void *context) {
    random_fixture_t *fixture = context;
    ++fixture->calls;
    return fixture->sample;
}

static uint32_t sequence_random(void *context) {
    random_fixture_t *fixture = context;
    ++fixture->calls;
    return fixture->sample++;
}

static void test_uniform(void) {
    uint8_t result = 0;
    random_fixture_t fixture = {0};
    assert(!dice_uniform(0, constant_random, &fixture, &result));
    assert(!dice_uniform(6, NULL, NULL, &result));
    assert(!dice_uniform(6, constant_random, &fixture, NULL));
    assert(!dice_uniform(6, constant_random, &fixture, &result));
    assert(fixture.calls == 32);
    fixture.calls = 0;
    assert(dice_uniform(6, sequence_random, &fixture, &result));
    assert(fixture.calls == 5 && result == 5);
    for (unsigned type = 0; type < DICE_TYPE_COUNT; ++type) {
        unsigned counts[101] = {0};
        const unsigned sides = dice_sides[type];
        fixture.sample = 100 * sides;
        fixture.calls = 0;
        for (unsigned index = 0; index < sides * 100; ++index) {
            assert(dice_uniform((uint8_t)sides, sequence_random, &fixture, &result));
            assert(result >= 1 && result <= sides);
            ++counts[result];
        }
        for (unsigned face = 1; face <= sides; ++face) assert(counts[face] == 100);
    }
}

static void test_controls(void) {
    dice_model_t model;
    dice_model_init(&model);
    assert(model.type == 2 && model.count == 1 && model.page == DICE_HOME);
    assert(dice_model_action(&model, DICE_OK_HOLD, 0, NULL, NULL));
    for (unsigned index = 0; index < 20; ++index) dice_model_action(&model, DICE_UP, 0, NULL, NULL);
    assert(model.count == DICE_MAX_COUNT);
    assert(dice_model_action(&model, DICE_OK, 0, NULL, NULL));
    for (unsigned index = 0; index < DICE_TYPE_COUNT; ++index) {
        assert(dice_model_action(&model, DICE_DOWN, 0, NULL, NULL));
    }
    assert(model.type == 2);
    dice_model_action(&model, DICE_UP, 0, NULL, NULL);
    dice_model_action(&model, DICE_UP, 0, NULL, NULL);
    assert(model.type == 0 && model.count == 1);
    dice_model_action(&model, DICE_OK_HOLD, 0, NULL, NULL);
    assert(!dice_model_action(&model, DICE_UP, 0, NULL, NULL));
    assert(!dice_model_action(&model, DICE_DOWN, 0, NULL, NULL));
    assert(model.count == 1);
}

static void test_rolls_and_history(void) {
    dice_model_t model;
    dice_model_init(&model);
    random_fixture_t fixture = {.sample = UINT32_MAX};
    dice_model_action(&model, DICE_DOWN_HOLD, 0, NULL, NULL);
    assert(!dice_model_action(&model, DICE_DOWN, 0, NULL, NULL));
    assert(model.history_cursor == 0);
    dice_model_action(&model, DICE_OK, 0, NULL, NULL);
    model.type = 7;
    model.count = 6;
    for (unsigned roll = 0; roll < 30; ++roll) {
        fixture.sample = 99 + 100 * roll;
        const uint64_t started = UINT64_C(4294967200) + roll * 1000;
        assert(dice_model_action(&model, DICE_OK, started, constant_random, &fixture));
        assert(model.result.total == 600);
        assert(model.rolling && !model.has_result);
        const dice_model_t snapshot = model;
        for (unsigned action = DICE_UP; action <= DICE_DOWN_HOLD; ++action) {
            assert(!dice_model_action(&model, (dice_action_t)action, started + 1,
                                      constant_random, &fixture));
        }
        assert(memcmp(&snapshot, &model, sizeof(model)) == 0);
        assert(!dice_model_tick(&model, started + DICE_ROLL_MS - 1));
        assert(dice_model_tick(&model, started + DICE_ROLL_MS));
        assert(!dice_model_tick(&model, started + DICE_ROLL_MS + 1));
        assert(model.has_result && !model.rolling);
        assert(model.history_count == (roll < 12 ? roll + 1 : 12));
        assert(model.history[0].total == 600);
    }
    for (unsigned roll = 0; roll < 15; ++roll) {
        fixture.sample = 100 + roll;
        dice_model_action(&model, DICE_OK, roll * 1000, constant_random, &fixture);
        dice_model_tick(&model, roll * 1000 + DICE_ROLL_MS);
    }
    for (unsigned index = 0; index < 12; ++index) {
        assert(model.history[index].total == (15 - index) * 6);
    }
    dice_model_action(&model, DICE_DOWN_HOLD, 0, NULL, NULL);
    for (unsigned index = 0; index < 50; ++index) dice_model_action(&model, DICE_DOWN, 0, NULL, NULL);
    assert(model.history_cursor == 11);
    for (unsigned index = 0; index < 50; ++index) dice_model_action(&model, DICE_UP, 0, NULL, NULL);
    assert(model.history_cursor == 0);
}

static void test_failure_atomicity(void) {
    dice_model_t model;
    dice_model_init(&model);
    random_fixture_t fixture = {.sample = 100};
    dice_model_action(&model, DICE_OK, 0, constant_random, &fixture);
    dice_model_tick(&model, 1000);
    const dice_result_t saved = model.result;
    fixture.sample = 0;
    assert(dice_model_action(&model, DICE_OK, 2000, constant_random, &fixture));
    assert(model.random_error && !model.rolling && model.history_count == 1);
    assert(memcmp(&saved, &model.result, sizeof(saved)) == 0);
}

int main(void) {
    test_uniform();
    test_controls();
    test_rolls_and_history();
    test_failure_atomicity();
    assert(dice_brightness(29999) == 70);
    assert(dice_brightness(30000) == 10);
    assert(dice_brightness(89999) == 10);
    assert(dice_brightness(90000) == 0);
    puts("Dice model: PASS (sampling, bounds, timing, history, failure atomicity, power)");
    return 0;
}
