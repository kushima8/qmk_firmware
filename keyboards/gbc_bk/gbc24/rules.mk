# MCU name
MCU = atmega32u4

# Bootloader selection
BOOTLOADER = atmel-dfu

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

SPLIT_KEYBOARD = no

# This is unnecessary for processing KC_MS_BTN*.
MOUSEKEY_ENABLE = yes

# Do not enable SLEEP_LED_ENABLE. it uses the same timer as BACKLIGHT_ENABLE
SLEEP_LED_ENABLE = no       # Breathing sleep LED during USB suspend

# To support OLED
OLED_ENABLE = yes               # Please Enable this in each keymaps.

# Disable other features to squeeze firmware size
SPACE_CADET_ENABLE = yes
MAGIC_ENABLE = yes

VIA_ENABLE = yes

GRAVE_ESC_ENABLE = yes

ENCODER_ENABLE = yes

ENCODER_MAP_ENABLE = yes

QMK_SETTINGS = yes 

TAP_DANCE_ENABLE = no
COMBO_ENABLE = no
KEY_OVERRIDE_ENABLE = no

DIP_SWITCH_ENABLE = yes

JOYSTICK_ENABLE = yes

