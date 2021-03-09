# Test firmware on soldering uzu42 keyboard

This firmware starts RGB lights in test animation mode always.
It helps testing LED on soldering.

When you are going to test OLED also, rewrite `OLED_DRIVER_ENABLE`'s value to
"yes" in rules.mk.
