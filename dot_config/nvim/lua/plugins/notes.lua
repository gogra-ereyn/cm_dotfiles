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
        daily_notes = {
            folder = "todos"
        },
    },
    config = function(_, opts)
        require("obsidian").setup(opts)
        vim.keymap.set("n", "<leader>od", "<cmd>Obsidian today<cr>")
        vim.keymap.set("n", "<leader>oy", "<cmd>Obsidian yesterday<cr>")
        vim.keymap.set("n", "<leader>ot", "<cmd>Obsidian tomorrow<cr>")
        vim.keymap.set("n", "<leader>oD", "<cmd>Obsidian dailies<cr>")

        vim.keymap.set("n", "<leader>oo", "<cmd>Obsidian quick_switch<cr>")
        vim.keymap.set("n", "<leader>os", "<cmd>Obsidian search<cr>")
        vim.keymap.set("n", "<leader>ob", "<cmd>Obsidian backlinks<cr>")
        vim.keymap.set("n", "<leader>on", "<cmd>Obsidian create<cr>")

        vim.keymap.set("v", "<leader>ol", "<cmd>Obsidian link<cr>")
    end,
}
