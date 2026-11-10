# gbc24 (RP2040版)

24キー + ロータリーエンコーダ + ポインティングデバイス(ジョイスティック / PMW3360トラックボール 交換式)のマクロパッド。
ATmega32u4 版 (`keyboards/gbc/gbc24`) を RP2040 (KB2040 互換ピン配置) に移植したもの。

* Keyboard Maintainer: [kushima8](https://github.com/kushima8)
* MCU: RP2040 / Bootloader: rp2040 (UF2)
* 対応マイコン: Adafruit KB2040 / RP2040 Kee Boar Driver (KB2040 互換ピン配置)

## ピン割り当て (AVR → RP2040 / KB2040換算)

Adafruit KB2040 のピン対応表に従って変換している。

| 機能 | AVR (Pro Micro) | RP2040 (KB2040) |
|---|---|---|
| ROW | D4, C6, D7, E6, B4 | GP4, GP5, GP6, GP7, GP8 |
| COL | B5, F4, F5, B6 | GP9, GP29, GP28, GP10 |
| エンコーダ A/B | D3 / D2 | GP0 / GP1 |
| ジョイスティック押し込み (信号/GND) | B2 / B1 | GP19 / GP18 |
| ジョイスティック X/Y | F6 / F7 (ADC) | GP27 / GP26 (ADC1/ADC0) |
| PMW3360 SCK/MOSI/MISO/NCS | B1/B2/B3/F7 | GP18/GP19/GP20/GP26 (SPI0) |

RP2040 の ADC は 12bit だが、QMK が 10bit に換算して返すため、`config.h` のジョイスティック可動域 (`JOY_X_MIN` / `JOY_X_MAX` / `JOY_Y_MIN` / `JOY_Y_MAX`、端の ADC 絶対値) は AVR 版と同じ値を使う。

`EXTRAKEY_ENABLE` / `NKRO_ENABLE` は AVR 版と同じく無効(`no`)のままにしている。

## ポインティング入力の切り替え (コンパイル時)

AVR 版と同じく `rules.mk` の【A】ジョイスティック /【B】トラックボール ブロックのコメントアウトで切り替える。

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
    OPT_DEFS += -DGBC_CORE_ENABLE

ビルド:

    make gbc_rp2040/gbc24:default

### trackball ビルドでの注意点

* ハードウェア SPI が GP18(SCK)/GP19(MOSI) を使用するため、**ジョイスティックの押し込みスイッチ (GP19, GND=GP18) は併用不可**。押し込みの読み取り (`gbc24.c` の `matrix_init_kb` / `duplex_scan_raw_post_kb`) は `JOYSTICK_ENABLE` 時のみコンパイルされる。
* `JOYSTICK_ENABLE = no` になるため、JOY_DIGI / JOY_MOUS / JOY_ANLG / JOY_SPD_UP / JOY_SPD_DN / JOY_DZ_UP / JOY_DZ_DN / JOY_DZ_RST キーコードは無効(キーマップは共通のままコンパイル可能)。
* センサーへのアクセスとトラックボール用キーコード (REC_RST など) の処理は `lib/gbc/gbc.c` が行う。gbc.c が custom pointing device driver を提供するため、`drivers/pmw3360/pmw3360_pointing_device_driver.c` は使わない。
* `GBC_CORE_ENABLE` は gbc24.c 側の軸補正を無効化するためのフラグ (gbc.c 側で同じ向きの補正を行うため、二重に補正しないよう必ず定義する)。
* RP2040 はフラッシュに余裕があるため LTO は不要。

## Bootloader / Bootmagic

ブートローダーに入る方法:

* **Physical reset button**: 基板上面のリセットボタンを押す(Caterina ブートローダーの Pro Micro は素早く2回押す。RP2040 も素早く2回押しで RPI-RP2 ドライブが出る)。
* **Keycode in layout**: `QK_BOOT` を割り当てたキーを押す。
* **Bootmagic Lite**: 左上キー (マトリクス (0,0)) を押しながら USB を接続する。**EEPROM が初期化され (Remap/Vial のキー配置、ジョイスティック設定、トラックボール設定が消える)**、そのままブートローダーに入る。RP2040 版は EEPROM 全体を消去するため、ジョイスティックのキャリブレーション値も消える (AVR 版では残る)。

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
