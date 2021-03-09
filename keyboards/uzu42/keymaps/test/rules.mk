# Link Time Optimization: for small binary
LTO_ENABLE = yes

# RGB lights test animation mode at start
RGBLIGHT_ENABLE = yes
SRC += rgbtest.c

# To test OLED too, rewrite "no" to "yes"
OLED_DRIVER_ENABLE = no
