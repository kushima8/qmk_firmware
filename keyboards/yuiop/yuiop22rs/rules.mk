MCU = atmega32u4

BOOTLOADER = caterina

# Not implemented yet.
## Split keyboard enabled
#SPLIT_KEYBOARD = yes
#SPLIT_TRANSPORT = mirror

# Build Options
LTO_ENABLE = yes

# Feature Options
BOOTMAGIC_ENABLE = no
MOUSEKEY_ENABLE = yes
EXTRAKEY_ENABLE = yes
CONSOLE_ENABLE = no
COMMAND_ENABLE = no
SLEEP_LED_ENABLE = no
COMBO_ENABLE = no
NKRO_ENABLE = no
BACKLIGHT_ENABLE = no
AUDIO_ENABLE = no
RGBLIGHT_ENABLE = no
RGB_MATRIX_ENABLE = no
LEADER_ENABLE = no
MIDI_ENABLE = no
UNICODE_ENABLE = no
BLUETOOTH_ENABLE = no

# Rotary switch works with DIP switch driver.
DIP_SWITCH_ENABLE = yes
