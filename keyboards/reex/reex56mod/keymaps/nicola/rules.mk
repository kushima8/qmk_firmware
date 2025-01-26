SRC += a2j/translate_ansi_to_jis.c
SRC += nicola/nicola.c
SRC += nicola/jtu.c
SRC += nicola/key_duration.c

# 16-bit timer (TIMER1) を使用したタイムアウト
# 一定時間経過すると、key down 状態のままでもキーが出力される
OPT_DEFS = -DTIMEOUT_INTERRUPT

# NICOLA key down timre requires TIMER1 so Backlight LED must be turned off
BACKLIGHT_ENABLE = no       # Enable keyboard backlight functionality
