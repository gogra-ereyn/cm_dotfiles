local mp = require "mp"
local utils = require "mp.utils"
local input = require "mp.input"

local function split_extension(filename)
    local stem
    local extension

    stem, extension = filename:match("^(.*)(%.[^.]*)$")

    if not stem or stem == "" then
        return filename, ""
    end

    return stem, extension
end

local function rename_current()
    local path
    local directory
    local filename
    local stem
    local extension

    path = mp.get_property("path")

    if not path or path == "" then
        mp.osd_message("no file loaded")
        return
    end

    if path:match("^%a[%w+.-]*://") then
        mp.osd_message("not a local file")
        return
    end

    path = mp.command_native({ "normalize-path", path })
    directory, filename = utils.split_path(path)
    stem, extension = split_extension(filename)

    input.get({
        prompt = "rename: ",
        default_text = stem,
        cursor_position = #stem + 1,

        submit = function(new_stem)
            local new_path
            local info
            local ok
            local err

            if not new_stem or new_stem == "" then
                mp.osd_message("rename cancelled")
                return
            end

            if new_stem:find("/", 1, true) then
                mp.osd_message("name cannot contain /")
                return
            end

            new_path = utils.join_path(directory, new_stem .. extension)

            if new_path == path then
                mp.osd_message("name unchanged")
                return
            end

            info = utils.file_info(new_path)

            if info then
                mp.osd_message("destination already exists")
                return
            end

            ok, err = os.rename(path, new_path)

            if not ok then
                mp.msg.error("rename failed: " .. tostring(err))
                mp.osd_message("rename failed")
                return
            end

            mp.msg.info(new_path)
            mp.osd_message("renamed: " .. new_stem .. extension)
        end,
    })
end

mp.add_key_binding(nil, "rename-current", rename_current)
