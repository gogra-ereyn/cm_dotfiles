local opt = vim.opt

opt.number=true
opt.relativenumber=true


opt.cindent = true
opt.cinoptions = ':0,l1,t0,g0,(0'

local function cbrace()
    vim.api.nvim_buf_set_keymap(0, 'i', '{<CR>', '{<CR>}<ESC>O', {noremap = true, silent = true})
    vim.api.nvim_buf_set_keymap(0, 'i', '}<CR>', '}<CR>', {noremap = true, silent = true})
    vim.api.nvim_buf_set_keymap(0, 'i', '{;', '{<CR>};<ESC>O', {noremap = true, silent = true})
    vim.api.nvim_buf_set_keymap(0, 'i', '{}', '{}<Left>', {noremap = true, silent = true})
    vim.api.nvim_buf_set_keymap(0, 'i', '{<Tab>', '{<CR>}<Esc>O', {noremap = true, silent = true})
end

vim.api.nvim_create_autocmd({"BufNewFile", "BufRead"}, {
    pattern = {"*.c", "*.h"},
    callback = cbrace
})

opt.tabstop = 4
opt.shiftwidth = 4
opt.softtabstop = 4
opt.expandtab = true

opt.termguicolors=true
opt.inccommand = "split"
opt.smartcase = true
opt.ignorecase = true

opt.undofile = true

opt.splitbelow = true
opt.splitright = true

--opt.signcolumn = "yes"
opt.shada = { "'10", "<0", "s10", "h" }

-- fuk folding. rly.
opt.foldenable = false
opt.foldmethod = 'manual'
opt.foldlevelstart = 99

--opt.wrap = false
-- remove useless edit suggestions

opt.wildmenu=true
opt.wildoptions="pum"
--opt.wildignorecase=true
opt.wildmode = 'longest:full,full'
opt.wildignore = '.hg,.svn,*~,*.png,*.jpg,*.gif,*.min.js,*.swp,*.o,vendor,dist,_site'

opt.ignorecase = true
opt.smartcase = true
opt.vb = true

opt.diffopt:append('iwhite')
opt.diffopt:append('algorithm:histogram')
opt.diffopt:append('indent-heuristic')

opt.colorcolumn = '100'

opt.shortmess = "A"
opt.listchars = {
    tab = '^ ',
    trail = '•',
    extends = '»',
    precedes = '«',
    nbsp='¬',
    --space = '·',
}


if vim.fn.executable("rg") then
    opt.grepprg = "rg --vimgrep --no-heading --smart-case"
    opt.grepformat = "%f:%l:%c:%m,%f:%l:%m"
end

opt.list=true

