# gbc24

24キー + ロータリーエンコーダ + ポインティングデバイス(ジョイスティック / PMW3360トラックボール 交換式)のマクロパッド。

* Keyboard Maintainer: [kushima8](https://github.com/kushima8)
* MCU: ATmega32u4 / Bootloader: atmel-dfu
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

## Bootloader

`BOOTMAGIC_ENABLE = no` のため Bootmagic reset は使えない。次のどちらかでブートローダーに入る。

* **Physical reset button**: 基板上面のリセットボタンを押す(Caterina ブートローダーの Pro Micro は素早く2回押す)。
* **Keycode in layout**: `QK_BOOT` を割り当てたキーを押す。

`rules.mk` は `BOOTLOADER = atmel-dfu` だが、一般的な Pro Micro には Caterina ブートローダーが書き込まれている。書き込む HEX ファイル自体はブートローダーの種類に依存しないが、`QK_BOOT` からブートローダーへの移行はブートローダーの種類によって動作が異なるため、うまくいかない場合は基板上面のリセットボタンを使う。
