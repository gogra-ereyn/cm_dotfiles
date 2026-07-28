local mp = require "mp"
local utils = require "mp.utils"

local home = os.getenv("HOME")
local data_home = os.getenv("XDG_DATA_HOME")
local directory
local playlist

mp.msg.info("keeper loaded")

if not data_home or data_home == "" then
	data_home = utils.join_path(home, ".local/share")
end

directory = utils.join_path(data_home, "mpv")
playlist = utils.join_path(directory, "keepers.m3u")

local function is_bookmarked(path)
	local file
	local line

	file = io.open(playlist, "r")
	if not file then
		return false
	end

	for line in file:lines() do
		if line == path then
			file:close()
			return true
		end
	end

	file:close()
	return false
end

local function bookmark_current()
	local path
	local name
	local result
	local file

	path = mp.get_property("path")
	name = mp.get_property("filename", "unknown")

	if not path or path == "" then
		mp.osd_message("no file loaded")
		return
	end

	if path:match("^%a[%w+.-]*://") then
		mp.osd_message("not a local file")
		return
	end

	path = mp.command_native({ "normalize-path", path })

	if not path then
		mp.osd_message("could not resolve path")
		return
	end

	if is_bookmarked(path) then
		mp.osd_message("already bookmarked: " .. name)
		return
	end

	mp.osd_message("bookmark dir: " .. directory, 5)

	file = io.open(playlist, "a")

	if not file then
		mp.osd_message("could not open bookmark file")
		return
	end

	file:write(path, "\n")
	file:close()

	mp.osd_message("bookmarked: " .. name)
    mp.msg.info("bookmarked: " .. path)
end

mp.add_key_binding("i", "bookmark-current", bookmark_current)
