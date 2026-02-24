#!/usr/bin/env python3

import i3ipc

BLOCKED = {"Brave-browser", "brave-browser"}

def on_fullscreen(i3, e):
    con = e.container
    if con and con.fullscreen_mode == 1 and con.window_class in BLOCKED:
        con.command("fullscreen disable")

i3 = i3ipc.Connection()
i3.on("window::fullscreen_mode", on_fullscreen)
i3.main()
