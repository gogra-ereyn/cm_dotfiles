return {
  {
    "dhruvasagar/vim-table-mode",
    ft = { "markdown", "text" },
    keys = {
      { "<Leader>tm", "<cmd>TableModeToggle<CR>", desc = "Toggle Table Mode" }
    },
    cmd = {
      "TableModeToggle",
      "TableModeEnable",
      "TableModeDisable"
    }
  }
}
