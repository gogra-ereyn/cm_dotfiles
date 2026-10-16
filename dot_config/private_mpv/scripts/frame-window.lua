local script = os.getenv("HOME") .. "/.local/bin/mpv-frame-window"

local function extract(mode)
    local path = mp.get_property("path")
    local pos = mp.get_property_number("time-pos")

    if not path or not pos then
        return
    end

    if path:match("^%a[%w+.-]*://") then
        mp.osd_message("frame extraction requires a local file")
        return
    end

    local duration = mode == "coarse" and 10 or 2
    local start = math.max(0, pos - duration / 2)

    mp.commandv("set", "pause", "yes")

    mp.commandv(
        "run",
        script,
        path,
        string.format("%.6f", start + duration / 2),
        mode
    )

    mp.osd_message("extracting " .. mode .. " frames...")
end

mp.register_script_message("frames-coarse", function()
    extract("coarse")
end)

mp.register_script_message("frames-dense", function()
    extract("dense")
end)
