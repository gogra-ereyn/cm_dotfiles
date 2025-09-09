local set = vim.keymap.set

set('n', '<C-Up>', ':resize +2<CR>', { silent = true })
set('n', '<C-Down>', ':resize -2<CR>', { silent = true })
set('n', '<C-Left>', ':vertical resize -2<CR>', { silent = true })
set('n', '<C-Right>', ':vertical resize +2<CR>', { silent = true })

set('n', '<C-h>', '<C-w>h', { noremap = true })
set('n', '<C-j>', '<C-w>j', { noremap = true })
set('n', '<C-k>', '<C-w>k', { noremap = true })
set('n', '<C-l>', '<C-w>l', { noremap = true })

-- keep cursor centered
set('n', '<C-d>', '<C-d>zz')
set('n', '<C-u>', '<C-u>zz')
set('n', 'n', 'nzz', { silent = true })
set('n', 'N', 'Nzz', { silent = true })
set('n', '*', '*zz', { silent = true })
set('n', '#', '#zz', { silent = true })
set('n', 'g*', 'g*zz', { silent = true })

---- make macro mispresses harder
set('n', '<leader>q', 'q')
set('n', 'q', '<Nop>')
set('n', "<leader>b", ":set invrelativenumber<CR>")

-- paste without adding selected to rm reg
set('x', "<leader>p", [["_dP]])
set('n', '<leader>il', ':set invlist<CR>')

set('n', '<leader>te', function()
    local current_expandtab = vim.bo.expandtab
    vim.bo.expandtab = not current_expandtab
    print("expandtab is now " .. (vim.bo.expandtab and "on" or "off"))
end, { noremap = true, silent = false, desc = "toggle expandtab" })

set('i', '<Tab>', '<Tab>', { noremap = true })

set("n", "<leader>e", ":e " .. vim.fn.expand("%:p:h") .. "/", { noremap = true, silent = false })
set("n", "<leader><leader>", "<C-^>", { noremap = true, silent = true })
