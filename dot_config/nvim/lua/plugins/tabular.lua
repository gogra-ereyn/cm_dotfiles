return {
    {
        'godlygeek/tabular',
        cmd = { "Tabularize", "Tab" },
        keys = {
            { "<leader>t=",  ":Tabularize /=<CR>",          mode = { "n", "v" }, desc = "Align by =" },
            { "<leader>t:",  ":Tabularize /:<CR>",          mode = { "n", "v" }, desc = "Align by :" },
            { "<leader>tm",  "<cmd>Tabularize /,\\s*/<cr>", mode = { "n", "v" }, desc = "tabularize on comma+space" },
            { "<leader>t\"", "<cmd>Tabularize /\"/<cr>",    mode = { "n", "v" }, desc = "tabularize on quote" },
            { "<leader>t,",  ":Tabularize /,<CR>",          mode = { "n", "v" }, desc = "Align by ," },
            { "<leader>t|",  ":Tabularize /|<CR>",          mode = { "n", "v" }, desc = "Align by |" },
            { "<leader>a//", ":Tabularize /\\/\\/<CR>",     mode = { "n", "v" }, desc = "Align by //" },
            { "<leader>a ",  ":Tabularize / <CR>",          mode = { "n", "v" }, desc = "Align by space" },
        },
    },
}
