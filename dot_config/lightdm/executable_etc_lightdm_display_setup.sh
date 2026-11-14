#!/bin/bash

xrandr --output DP-2 --mode 3840x2160 --rate 240 --output DP-3 --mode 3840x2160 --rate 240 --primary --right-of DP-2 --output DP-1 --off || true

