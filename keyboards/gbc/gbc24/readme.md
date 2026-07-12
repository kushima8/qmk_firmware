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

    # 【B】PMW3360 トラックボール構成
    JOYSTICK_ENABLE = no
    POINTING_DEVICE_ENABLE = yes
    POINTING_DEVICE_DRIVER = custom
    SRC += drivers/pmw3360/pmw3360.c
    SRC += drivers/pmw3360/pmw3360_pointing_device_driver.c
    QUANTUM_LIB_SRC += spi_master.c
    DIP_SWITCH_ENABLE = no
    LTO_ENABLE = yes

書き換え後は通常どおりビルドする:

    make gbc/gbc24:default

### trackball ビルドでの注意点

* ハードウェア SPI が B1(SCK)/B2(MOSI) を使用するため、**DIP スイッチ (B2, GND=B1) は併用不可**のため【B】ブロックで無効化している。
* `JOYSTICK_ENABLE = no` になるため、JOY_DIGI / JOY_MOUS / JOY_ANLG / JOY_SPD_UP / JOY_SPD_DN キーコードは無効(キーマップは共通のままコンパイル可能)。
* トラックボールのカーソル移動はマウス HID として送信される。クリックは KC_MS_BTN1 等(MOUSEKEY)で行う。
* フラッシュ節約のため LTO_ENABLE = yes を有効にする(【B】ブロックに含まれる)。
* PMW3360 ドライバは同梱の `drivers/pmw3360/` を使用(NCS デフォルト = F7)。
* 本ドライバ差し替えにより REEX 系キーボードはコンパイル不可になる場合がある(許容)。

## Bootloader

Enter the bootloader in 3 ways:

* **Bootmagic reset**: Hold down the key at (0,0) in the matrix and plug in the keyboard
* **Physical reset button**: Briefly press the button on the back of the PCB
* **Keycode in layout**: Press the key mapped to `QK_BOOT` if it is available
