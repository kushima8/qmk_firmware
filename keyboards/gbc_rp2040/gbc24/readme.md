# gbc24 (RP2040版)

24キー + ロータリーエンコーダ + ポインティングデバイス(ジョイスティック / PMW3360トラックボール 交換式)のマクロパッド。
ATmega32u4 版 (`keyboards/gbc/gbc24`) を RP2040 (KB2040 互換ピン配置) に移植したもの。

* Keyboard Maintainer: [kushima8](https://github.com/kushima8)
* MCU: RP2040 / Bootloader: rp2040 (UF2)

## ピン割り当て (AVR → RP2040 / KB2040換算)

hook → hook_rp2040 と同じ Adafruit KB2040 のピン対応表に従って変換している。

| 機能 | AVR (Pro Micro) | RP2040 (KB2040) |
|---|---|---|
| ROW | D4, C6, D7, E6, B4 | GP4, GP5, GP6, GP7, GP8 |
| COL | B5, F4, F5, B6 | GP9, GP29, GP28, GP10 |
| エンコーダ A/B | D3 / D2 | GP0 / GP1 |
| DIPスイッチ (信号/GND) | B2 / B1 | GP19 / GP18 |
| ジョイスティック X/Y | F6 / F7 (ADC) | GP27 / GP26 (ADC1/ADC0) |
| PMW3360 SCK/MOSI/MISO/NCS | B1/B2/B3/F7 | GP18/GP19/GP20/GP26 (SPI0) |

RP2040 は USB エンドポイントに余裕があるため `EXTRAKEY_ENABLE` / `NKRO_ENABLE` を有効化している。

## ポインティング入力の切り替え (コンパイル時)

AVR 版と同じく `rules.mk` の【A】ジョイスティック /【B】トラックボール ブロックのコメントアウトで切り替える。

* trackball ビルドではハードウェア SPI0 が GP18(SCK)/GP19(MOSI) を使うため、**DIP スイッチ (GP19, GND=GP18) は併用不可**(【B】ブロックで無効化)。
* RP2040 はフラッシュに余裕があるため LTO は不要。

ビルド:

    make gbc_rp2040/gbc24:default

## Bootloader

* **Physical reset**: BOOT ボタンを押しながら USB 接続 (または RESET をダブルタップ: `RP2040_BOOTLOADER_DOUBLE_TAP_RESET` 有効)
* **Keycode in layout**: `QK_BOOT` を割り当てたキー

UF2 ファイルを RPI-RP2 ドライブへコピーして書き込む。
