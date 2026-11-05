# gbc24

24キー + ロータリーエンコーダ + ポインティングデバイス(ジョイスティック / PMW3360トラックボール 交換式)のマクロパッド。

* Keyboard Maintainer: [kushima8](https://github.com/kushima8)
* MCU: ATmega32u4 / Bootloader: caterina (Pro Micro)
* ベースファームウェア: [kushima8/qmk_firmware](https://github.com/kushima8/qmk_firmware) (QMK 0.22.14ベース)

## ポインティング入力の切り替え (コンパイル時)

ジョイスティック接続部を交換して PMW3360 センサ搭載トラックボールを装着できる。
どちらを使うかは **rules.mk 内のブロックのコメントアウト** で切り替える。

| 設定値 | 構成 | 使用ピン |
|---|---|---|
| `joystick` (デフォルト) | アナログジョイスティック(DIGI/MOUS/ANLG 3モード) | X=F6, Y=F7 (ADC) |
| `trackball` | PMW3360 トラックボール | SCK=B1, MOSI=B2, MISO=B3, NCS=F7 |

### 切り替え方法

`keyboards/gbc/gbc24/rules.mk` に【A】ジョイスティック構成と【B】トラックボール構成の2ブロックがある。

* **ジョイスティックを使う**: 【A】を有効化し、【B】を全てコメントアウト(デフォルト状態)
* **トラックボールを使う**: 【A】を全てコメントアウトし、【B】の `#` を外す

トラックボール構成の例:

    # 【A】アナログジョイスティック構成 (デフォルト)
    #JOYSTICK_ENABLE = yes

    # 【B】PMW3360 トラックボール構成 (lib/gbc 経由)
    JOYSTICK_ENABLE = no
    POINTING_DEVICE_ENABLE = yes
    POINTING_DEVICE_DRIVER = custom
    SRC += drivers/pmw3360/pmw3360.c
    SRC += lib/gbc/gbc.c
    QUANTUM_LIB_SRC += spi_master.c
    LTO_ENABLE = yes
    OPT_DEFS += -DGBC_CORE_ENABLE

書き換え後は通常どおりビルドする:

    make gbc/gbc24:default

### trackball ビルドでの注意点

* ハードウェア SPI が B1(SCK)/B2(MOSI) を使用するため、**ジョイスティックの押し込みスイッチ (B2, GND=B1) は併用不可**。押し込みの読み取り (`gbc24.c` の `matrix_init_kb` / `duplex_scan_raw_post_kb`) は `JOYSTICK_ENABLE` 時のみコンパイルされる。
* `JOYSTICK_ENABLE = no` になるため、JOY_DIGI / JOY_MOUS / JOY_ANLG / JOY_SPD_UP / JOY_SPD_DN / JOY_DZ_UP / JOY_DZ_DN / JOY_DZ_RST キーコードは無効(キーマップは共通のままコンパイル可能)。
* センサーへのアクセスとトラックボール用キーコード (REC_RST など) の処理は `lib/gbc/gbc.c` が行う。gbc.c が custom pointing device driver を提供するため、`drivers/pmw3360/pmw3360_pointing_device_driver.c` は使わない。
* `GBC_CORE_ENABLE` は gbc24.c 側の軸補正を無効化するためのフラグ (gbc.c 側で同じ向きの補正を行うため、二重に補正しないよう必ず定義する)。
* トラックボールのカーソル移動はマウス HID として送信される。クリックは KC_MS_BTN1 等(MOUSEKEY)で行う。
* フラッシュ節約のため LTO_ENABLE = yes を有効にする(【B】ブロックに含まれる)。
* PMW3360 ドライバは同梱の `drivers/pmw3360/` を使用(NCS デフォルト = F7)。
* 本ドライバ差し替えにより REEX 系キーボードはコンパイル不可になる場合がある(許容)。

## Bootloader / Bootmagic

ブートローダーに入る方法:

* **Physical reset button**: 基板上面のリセットボタンを押す(Caterina ブートローダーの Pro Micro は素早く2回押す。RP2040 も素早く2回押しで RPI-RP2 ドライブが出る)。
* **Keycode in layout**: `QK_BOOT` を割り当てたキーを押す。
* **Bootmagic Lite**: 左上キー (マトリクス (0,0)) を押しながら USB を接続する。**EEPROM が初期化され (Remap/Vial のキー配置、ジョイスティック設定、トラックボール設定が消える)**、そのままブートローダーに入る。ジョイスティックのキャリブレーション値だけは消えない (消すには `JOY_CALIB` 2 秒長押し)。

`rules.mk` の `BOOTMAGIC_ENABLE` は `VIA_ENABLE = yes` によって強制的に `yes` にされるため、`no` と書いても無効化できない。
## 機能セットの統一

QMK(Remap/VIA) 版・Vial 版、AVR 版・RP2040 版の 4 構成で機能差を出さないため、
マニュアルに記載のない次の機能はすべての構成で無効にしている (AVR 版のフラッシュ節約も兼ねる)。

* Vial コンボ (`COMBO_ENABLE = no`)。VIA は元々非対応、Vial AVR 版はフラッシュに入らない
* Magic キーコード (`MAGIC_ENABLE = no`)。`keycode_config()` / `mod_config()` はスタブ化 (gbc24.c / lib/gbc/gbc.c)
* Space Cadet、Grave Esc、メカニカルロックキー (`LOCKING_SUPPORT_ENABLE`)
* Vial QMK Settings (`QMK_SETTINGS = no`)

ジョイスティックのキャリブレーション (`JOY_CALIB`) は 4 構成すべてで有効。保存先は EEPROM 末尾の
16 バイトで、`DYNAMIC_KEYMAP_EEPROM_MAX_ADDR` を同じ量だけ手前に下げて確保している
(VIA/Vial のマクロバッファが 16 バイト縮むだけで、既存のキーマップ / マクロの位置は変わらない)。
