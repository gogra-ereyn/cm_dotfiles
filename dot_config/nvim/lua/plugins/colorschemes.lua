return {
    {
        'lirael',
        dir = vim.fn.stdpath("config") .. "/lua/lirael",
        dependencies = { "rktjmp/lush.nvim" },
        lazy = false,
        priority = 1000,
        config = function()
            vim.cmd(":colorscheme lirael")
        end,
    },
}
