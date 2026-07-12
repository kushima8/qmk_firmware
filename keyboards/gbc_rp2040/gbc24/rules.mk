# MCU name
MCU = RP2040

# Bootloader selection
BOOTLOADER = rp2040

# Build Options
BOOTMAGIC_ENABLE = no       # Enable Bootmagic Lite
EXTRAKEY_ENABLE = yes       # Audio control and System control (RP2040 has enough USB endpoints)
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

SRC += analog.c

DIP_SWITCH_ENABLE = yes

# =====================================================================
# ポインティング入力の選択 (コメントアウトで切り替え)
#
#   ジョイスティックを使う場合  : 【A】を有効化、【B】を全てコメントアウト
#   トラックボールを使う場合    : 【A】を全てコメントアウト、【B】を有効化
#
# ピン割り当て:
#   joystick  : X=F6, Y=F7 (ADC)
#   trackball : SCK=B1, MOSI=B2, MISO=B3, NCS=F7 (ハードウェアSPI)
# =====================================================================

# ---------------------------------------------------------------------
# 【A】アナログジョイスティック構成 (デフォルト)
# ---------------------------------------------------------------------
JOYSTICK_ENABLE = yes

# ---------------------------------------------------------------------
# 【B】PMW3360 トラックボール構成
#     ※ B1(SCK)/B2(MOSI) をSPIが使うため DIP スイッチは併用不可 (= no にする)
#     ※ RP2040 はフラッシュに余裕があるため LTO は不要
# ---------------------------------------------------------------------
#JOYSTICK_ENABLE = no
#POINTING_DEVICE_ENABLE = yes
#POINTING_DEVICE_DRIVER = custom
#SRC += drivers/pmw3360/pmw3360.c
#SRC += drivers/pmw3360/pmw3360_pointing_device_driver.c
#QUANTUM_LIB_SRC += spi_master.c
#DIP_SWITCH_ENABLE = no

# lib/joystick_user is located one level up from the keyboard dir:
#   keyboards/gbc/
#   ├── gbc24/                (this keyboard)
#   └── lib/joystick_user/
# ソース全体が #ifdef JOYSTICK_ENABLE で括られているため、
# trackball ビルドでも追加したまま(空にコンパイルされる)で問題ない。
SRC      += joystick_user.c
VPATH    += keyboards/gbc_rp2040/lib/joystick_user
OPT_DEFS += -Ikeyboards/gbc_rp2040/lib/joystick_user

