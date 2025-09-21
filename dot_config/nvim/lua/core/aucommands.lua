local function trim_whitespace()
    local save = vim.fn.winsaveview()
    vim.cmd([[keeppatterns %s/\s\+$//e]])
    vim.fn.winrestview(save)
end

local tsg = vim.api.nvim_create_augroup("TrimWhiteSpace", { clear = true })

vim.api.nvim_create_autocmd("BufWritePre", {
    group = tsg,
    callback = function()
        local excl = { "markdown", "text" }
        local ct = vim.bo.filetype
        if not vim.tbl_contains(excl, ct) then
            trim_whitespace()
        end
    end,
})

vim.api.nvim_create_autocmd(
    'BufReadPost',
    {
        pattern = '*',
        callback = function(_)
            if vim.fn.line("'\"") > 1 and vim.fn.line("'\"") <= vim.fn.line("$") then
                if not vim.fn.expand('%:p'):find('.git', 1, true) then
                    vim.cmd('exe "normal! g\'\\""')
                end
            end
        end
    }
)

vim.api.nvim_create_autocmd("FileType", {
    pattern = { "c", "h" },
    callback = function()
        vim.bo.expandtab = false
        vim.bo.tabstop = 8
        vim.bo.shiftwidth = 8
    end
})

vim.api.nvim_create_autocmd("FileType", {
    pattern = { "cpp", "hpp" },
    callback = function()
        vim.bo.expandtab = true
        vim.bo.tabstop = 4
        vim.bo.shiftwidth = 4
    end
})

local text = vim.api.nvim_create_augroup('text', { clear = true })
for _, pat in ipairs({ 'text', 'markdown', 'mail', 'gitcommit' }) do
    vim.api.nvim_create_autocmd('Filetype', {
        pattern = pat,
        group = text,
        command = 'setlocal spell tw=72 colorcolumn=73',
    })
end
