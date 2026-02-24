#!/usr/bin/env python3

import i3ipc

BLOCKED_CLASSES = {"Brave-browser", "brave-browser"}

def on_fullscreen(i3, e):
    if e.container and e.container.window_class in BLOCKED_CLASSES:
        if e.container.fullscreen_mode == 1:
            e.container.command("fullscreen disable")

i3 = i3ipc.Connection()
i3.on("window::fullscreen_mode", on_fullscreen)
i3.main()
