idf_component_register(
    SRCS "main.c"
         "dice_model.c"
         "dice_ui.c"
         "../assets/fonts/dice_font_16.c"
    INCLUDE_DIRS "."
    REQUIRES bsp esp_hw_support esp_timer
)
