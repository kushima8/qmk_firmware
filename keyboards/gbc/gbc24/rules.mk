# MCU name
MCU = atmega32u4

# Bootloader selection
BOOTLOADER = caterina

# Build Options
BOOTMAGIC_ENABLE = yes      # VIA_ENABLE により強制 yes (no と書いても効かない)。左上キー (0,0) を押しながら USB 接続で EEPROM 初期化 + ブートローダー
EXTRAKEY_ENABLE = yes       # メディアキー (音量など)。ATmega32u4 でもエンドポイントは足りる (KBD/MOUSE/EXTRA/RAW x2/JOYSTICK = 6)
CONSOLE_ENABLE = no         # Console for debug
COMMAND_ENABLE = no         # Commands for debug and configuration
NKRO_ENABLE = no            # Enable N-Key Rollover
BACKLIGHT_ENABLE = no       # Enable keyboard backlight functionality
AUDIO_ENABLE = no           # Audio output

# Duplex matrix.
CUSTOM_MATRIX = lite
SRC += lib/duplexmatrix/duplexmatrix.c

SPLIT_KEYBOARD = no

# KC_MS_BTN* の処理と、ジョイスティックのマウスモードでのドラッグ
# (mousekey_get_report でボタン状態を引き継ぐ) に必要。no にするとドラッグが切れる。
MOUSEKEY_ENABLE = yes

# Do not enable SLEEP_LED_ENABLE. it uses the same timer as BACKLIGHT_ENABLE
SLEEP_LED_ENABLE = no       # Breathing sleep LED during USB suspend

# To support OLED
OLED_ENABLE = yes               # 液晶付きモデル用 (keyboard 側で有効化)

# 4 構成で機能差を出さないため、マニュアル未記載の機能は無効にする
SPACE_CADET_ENABLE = no  # 4 構成 (QMK/Vial x AVR/RP2040) で機能差を出さないため無効 (AVR のフラッシュ節約も兼ねる)
MAGIC_ENABLE = no  # 4 構成 (QMK/Vial x AVR/RP2040) で機能差を出さないため無効 (AVR のフラッシュ節約も兼ねる)。keycode_config/mod_config は gbc24.c / gbc.c でスタブ化

# AVR 版はフラッシュが厳しいため常に有効 (サイズの実測値は各 readme のビルド確認欄を参照)
LTO_ENABLE = yes

VIA_ENABLE = yes

GRAVE_ESC_ENABLE = no  # 4 構成 (QMK/Vial x AVR/RP2040) で機能差を出さないため無効 (AVR のフラッシュ節約も兼ねる)

ENCODER_ENABLE = yes

ENCODER_MAP_ENABLE = yes

QMK_SETTINGS = no  # Vial 専用オプション。QMK Settings を有効にすると keycode_config のスタブが外れ AVR に入らなくなる

TAP_DANCE_ENABLE = no
COMBO_ENABLE = no  # 4 構成 (QMK/Vial x AVR/RP2040) で機能差を出さないため無効 (AVR のフラッシュ節約も兼ねる)。VIA は非対応、Vial AVR はフラッシュに入らない
KEY_OVERRIDE_ENABLE = no

SRC += analog.c

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
# 【B】PMW3360 トラックボール構成 (lib/gbc 経由)
#     ※ センサーへのアクセスは lib/gbc/gbc.c が行う。
#       gbc.c が custom pointing device driver
#       (pointing_device_driver_init / get_report / get_cpi / set_cpi)
#       を提供するため、pmw3360_pointing_device_driver.c は使わない。
#     ※ pmw3360.c は同梱ドライバ (drivers/pmw3360/) を無修正で使用。
#     ※ B1(SCK)/B2(MOSI) をSPIが使うため、ジョイスティック押し込み
#       (gbc24.c の duplex_scan_raw_post_kb, JOYSTICK_ENABLE 時のみ有効) は使えない
# ---------------------------------------------------------------------
#JOYSTICK_ENABLE = no
#POINTING_DEVICE_ENABLE = yes
#POINTING_DEVICE_DRIVER = custom
#SRC += drivers/pmw3360/pmw3360.c
#SRC += lib/gbc/gbc.c
#QUANTUM_LIB_SRC += spi_master.c
# gbc.c 使用時のフラグ (gbc24.c 側の軸補正を無効化するために使用)
#OPT_DEFS += -DGBC_CORE_ENABLE

# lib/joystick_user is located one level up from the keyboard dir:
#   keyboards/gbc/
#   ├── gbc24/                (this keyboard)
#   └── lib/joystick_user/
# ソース全体が #ifdef JOYSTICK_ENABLE で括られているため、
# trackball ビルドでも追加したまま(空にコンパイルされる)で問題ない。
SRC      += joystick_user.c
VPATH    += keyboards/gbc/lib/joystick_user
OPT_DEFS += -Ikeyboards/gbc/lib/joystick_user

