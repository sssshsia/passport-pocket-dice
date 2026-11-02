#pragma once

#include "dice_model.h"
#include <stdbool.h>
#include <stdint.h>

bool dice_ui_font_check(void);
void dice_ui_create(void);
void dice_ui_render(const dice_model_t *model, int battery, uint64_t now_ms, bool buttons_ok);
