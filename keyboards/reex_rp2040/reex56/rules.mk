# MCU name
MCU = RP2040

# Bootloader selection
BOOTLOADER = rp2040

# Link Time Optimization required for size.
LTO_ENABLE = yes

# Build Options
BOOTMAGIC_ENABLE = no       # Enable Bootmagic Lite
EXTRAKEY_ENABLE = yes       # Audio control and System control
CONSOLE_ENABLE = no         # Console for debug
COMMAND_ENABLE = no         # Commands for debug and configuration
NKRO_ENABLE = yes           # Enable N-Key Rollover
BACKLIGHT_ENABLE = no       # Enable keyboard backlight functionality
AUDIO_ENABLE = no           # Audio output

# Duplex matrix.
CUSTOM_MATRIX = lite
SRC += lib/duplexmatrix/duplexmatrix.c
# Reex56Mod is split keyboard.
SPLIT_KEYBOARD = yes

# Optical sensor driver for trackball.
POINTING_DEVICE_ENABLE = yes
POINTING_DEVICE_DRIVER = custom
SRC += drivers/pmw3360/pmw3360.c
QUANTUM_LIB_SRC += spi_master.c # Optical sensor use SPI to communicate

# This is unnecessary for processing KC_MS_BTN*.
MOUSEKEY_ENABLE = yes

# Enabled only one of RGBLIGHT and RGB_MATRIX if necessary.
RGBLIGHT_ENABLE = yes       # Enable RGBLIGHT

# Do not enable SLEEP_LED_ENABLE. it uses the same timer as BACKLIGHT_ENABLE
SLEEP_LED_ENABLE = no       # Breathing sleep LED during USB suspend

# To support OLED
OLED_ENABLE = yes               # Please Enable this in each keymaps.
SRC += lib/oledkit/oledkit.c    # OLED utility for Reex series.

# Include common library
SRC += lib/reex/reex.c

# Disable other features to squeeze firmware size
SPACE_CADET_ENABLE = yes
MAGIC_ENABLE = yes

VIA_ENABLE = yes

GRAVE_ESC_ENABLE = yes

WS2812_DRIVER = vendor
SERIAL_DRIVER = vendor

QMK_SETTINGS = yes 

TAP_DANCE_ENABLE = yes
COMBO_ENABLE = yes
KEY_OVERRIDE_ENABLE = yes