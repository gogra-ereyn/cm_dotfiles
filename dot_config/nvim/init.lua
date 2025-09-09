vim.g.mapleader = " "
vim.g.maplocalleader = " "
vim.opt.mouse = ""

local lazypath = vim.fn.stdpath "data" .. "/lazy/lazy.nvim"
if not vim.uv.fs_stat(lazypath) then
    vim.fn.system {
        "git",
        "clone",
        "--filter=blob:none",
        "https://github.com/folke/lazy.nvim.git",
        "--branch=stable",
        lazypath,
    }
end

vim.opt.rtp:prepend(lazypath)
require("lazy").setup({ import = "plugins" }, {
    change_detection = {
        notify = false,
    },
})

require("core.binds")
require("core.options")
require("core.commands")
require("core.aucommands")


require("core.shada").setup()

