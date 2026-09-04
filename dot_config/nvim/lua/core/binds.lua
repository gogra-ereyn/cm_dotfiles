local set = vim.keymap.set

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
set('n', 'g;', 'g;zz', { silent = true })
set('n', 'g,', 'g,zz', { silent = true })

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
set("i", "<C-Left>", "└─", { noremap = true })
set("i", "<C-Up>", "│", { noremap = true })
set("i", "<C-Right>", "├─", { noremap = true })
set("i", "<C-Down>", "─", { noremap = true })


local function jumpywumpy(filter, dir)
    local jumps, pos = unpack(vim.fn.getjumplist())
    local cur = vim.api.nvim_get_current_buf()
    local i = pos + 1 + dir
    while i >= 1 and i <= #jumps do
        local b = jumps[i].bufnr
        if filter(b, cur) and vim.api.nvim_buf_is_valid(b) then
            local delta = i - (pos + 1)
            local key = delta < 0 and "<C-o>" or "<C-i>"
            vim.cmd("normal! " .. math.abs(delta)
                .. vim.api.nvim_replace_termcodes(key, true, false, true))
            return
        end
        i = i + dir
    end
end

local same = function(b, c) return (b == c) end
local diff = function(b, c) return (b ~= c) end

set("n", "<leader>o", function() jumpywumpy(diff, -1) end)
set("n", "<leader>i", function() jumpywumpy(diff, 1) end)
set("n", "<leader>o", function() jumpywumpy(diff, -1) end)
set("n", "<leader>i", function() jumpywumpy(diff, 1) end)

-- checkbox binds

local function insert_checkbox()
    local line = vim.api.nvim_get_current_line()

    if line:match("^%s*$") then
        return "- [ ] "
    end

    return "<CR>- [ ] "
end

set("i", "<C-m>", insert_checkbox, {
    expr = true,
    desc = "new checkbox",
})

set("n", "<C-m>", "o- [ ] ", {
    desc = "new checkbox",
})


set("n", "<C-/>", "i/**/<Left><Left>", { noremap = true, silent = true })
set("i", "<C-/>", "/**/<Left><Left>", { noremap = true, silent = true })
