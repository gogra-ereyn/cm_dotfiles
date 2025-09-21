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

set('n', '?', '?\\v')
set('n', '/', '/\\v')
set('c', '%s/', '%sm/')


set('n', 'j', 'gj')
set('n', 'k', 'gk')

-- same dir as current buf
set('n', '<leader>o', ':e <C-R>=expand("%:p:h") . "/" <cr>')


-- jon-gj 's proximity sort.
-- can be installed via `cargo install proximity-sort`
set('', '<C-p>', function()
    opts = {}
    opts.cmd = 'fd --color=never --hidden --type f --type l --exclude .git'
    local base = vim.fn.fnamemodify(vim.fn.expand('%'), ':h:.:S')
    if base ~= '.' then
        opts.cmd = opts.cmd .. (" | proximity-sort %s"):format(vim.fn.shellescape(vim.fn.expand('%')))
    end
    opts.fzf_opts = {
        ['--scheme']   = 'path',
        ['--tiebreak'] = 'index',
        ["--layout"]   = "default",
    }
    require 'fzf-lua'.files(opts)
end)

set('n', '<leader>;', function()
    require 'fzf-lua'.buffers({
        fzf_opts = {
            ["--with-nth"]     = "{-3..-2}",
            ["--nth"]          = "-1",
            ["--delimiter"]    = "[:\u{2002}]",
            ["--header-lines"] = "false",
        },
        header = false,
    })
end)
