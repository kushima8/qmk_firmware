SRC += kaoriya.c matrixmask.c

# Link Time Optimization: for small binary
LTO_ENABLE = yes

# Mouse keys
MOUSEKEY_ENABLE = yes

# Audio control and System control: for Vol+, Vol-, and Mute
EXTRAKEY_ENABLE = yes

# RGB lights enable
RGBLIGHT_ENABLE = yes

# Enable OLED
OLED_DRIVER_ENABLE = yes
SRC += oled.c
