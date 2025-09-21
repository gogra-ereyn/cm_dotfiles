return {
    {
        'godlygeek/tabular',
        cmd = { "Tabularize", "Tab" },
        keys = {
            { "<leader>t=",  ":Tabularize /=<CR>",      mode = { "n", "v" }, desc = "Align by =" },
            { "<leader>t:",  ":Tabularize /:<CR>",      mode = { "n", "v" }, desc = "Align by :" },
            { "<leader>t,",  ":Tabularize /,<CR>",      mode = { "n", "v" }, desc = "Align by ," },
            { "<leader>t|",  ":Tabularize /|<CR>",      mode = { "n", "v" }, desc = "Align by |" },
            { "<leader>a//", ":Tabularize /\\/\\/<CR>", mode = { "n", "v" }, desc = "Align by //" },
            { "<leader>a ",  ":Tabularize / <CR>",      mode = { "n", "v" }, desc = "Align by space" },
        },
    },
}
