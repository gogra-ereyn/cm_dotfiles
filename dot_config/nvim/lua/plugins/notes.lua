return {
  "obsidian-nvim/obsidian.nvim",
  version = "*",
  ft = "markdown",
  ---@module 'obsidian'
  ---@type obsidian.config
  opts = {
    legacy_commands = false,
    workspaces = {
      {
        name = "personal",
        path = "/apps/obsidian/myvault",
      }
    },
  },
  config=function()
    vim.keymap.set("n", "<leader>od", "<cmd>ObsidianToday<cr>")
    vim.keymap.set("n", "<leader>oy", "<cmd>ObsidianYesterday<cr>")
    vim.keymap.set("n", "<leader>ot", "<cmd>ObsidianTomorrow<cr>")
    vim.keymap.set("n", "<leader>oD", "<cmd>ObsidianDailies<cr>")

    vim.keymap.set("n", "<leader>oo", "<cmd>ObsidianQuickSwitch<cr>")
    vim.keymap.set("n", "<leader>os", "<cmd>ObsidianSearch<cr>")
    vim.keymap.set("n", "<leader>ob", "<cmd>ObsidianBacklinks<cr>")
    vim.keymap.set("n", "<leader>on", "<cmd>ObsidianNew<cr>")

    vim.keymap.set("v", "<leader>ol", "<cmd>ObsidianLink<cr>")
    vim.keymap.set("v", "<leader>oL", "<cmd>ObsidianLinkNew<cr>")
end
}
