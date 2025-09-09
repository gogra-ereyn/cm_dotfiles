return {
    {
        "sindrets/diffview.nvim",
        dependencies = { "nvim-lua/plenary.nvim" },
        config = function()
            require("diffview").setup()
            vim.keymap.set('n', '<leader>gd', ':DiffviewOpen<CR>')
            vim.keymap.set('n', '<leader>gh', ':DiffviewFileHistory %<CR>')
        end
    }
}
