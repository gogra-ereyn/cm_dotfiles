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
set('n', '<C-i>', '<C-i>zz')
set('n', '<C-o>', '<C-o>zz')
set('n', 'n', 'nzz', { silent = true })
set('n', 'N', 'Nzz', { silent = true })
set('n', '*', '*zz', { silent = true })
set('n', '#', '#zz', { silent = true })
set('n', 'g*', 'g*zz', { silent = true })

---- make macro mispresses harder

vim.keymap.set("n", "<leader>q", function()
  vim.cmd("normal! q")
end, { silent = true })
set("n", "q", "<Nop>", { silent = true })
set('n', "<leader>b", ":set invrelativenumber<CR>")

set('n', '<leader>il', ':set invlist<CR>')

set('n', '<leader>te', function()
    local current_expandtab = vim.bo.expandtab
    vim.bo.expandtab = not current_expandtab
    print("expandtab is now " .. (vim.bo.expandtab and "on" or "off"))
end, { noremap = true, silent = false, desc = "toggle expandtab" })

set('n', '<leader>sc', function()
    vim.opt.wildignorecase = not vim.opt.wildignorecase:get()
    print("wildignorecase: " .. tostring(vim.opt.wildignorecase:get()))
end, { desc = "toggle wildignorecase" })

set('i', '<Tab>', '<Tab>', { noremap = true })
set("n", "<leader>e", ":e " .. vim.fn.expand("%:p:h") .. "/", { noremap = true, silent = false })
set("n", "<leader><leader>", "<C-^>", { noremap = true, silent = true })


set('', 'H', '^')
set('', 'L', '$')

-- regex escaping ergo
set('n', 'j', 'gj')
set('n', 'k', 'gk')

-- same dir as current buf
set('n', '<leader>p', ':e <C-R>=expand("%:p:h") . "/" <cr>')

-- arrows for the lazy. mostly for annotating
-- code in markdown/cmts
set("i", "<C-Left>",  "└─", { noremap = true })
set("i", "<C-Up>",    "│",  { noremap = true })
set("i", "<C-Right>", "├─", { noremap = true })
set("i", "<C-Down>",  "─",  { noremap = true })
