return {
    {
        'ibhagwan/fzf-lua',
        config = function()
            -- stop putting a giant window over my editor
            require 'fzf-lua'.setup {
                winopts = {
                    split = "belowright 10new",
                    preview = {
                        hidden = true,
                    }
                },
                files = {
                    -- file icons are distracting
                    file_icons = false,
                    -- git icons are nice
                    git_icons = true,
                    -- but don't mess up my anchored search
                    _fzf_nth_devicons = true,
                },
                buffers = {
                    file_icons = false,
                    git_icons = true,
                },
                fzf_opts = {
                    ["--layout"] = "default",
                },
            }

            require 'fzf-lua'.register_ui_select()

            -- jon-gj 's proximity sort.
            -- can be installed via `cargo install proximity-sort`
            vim.keymap.set('', '<C-p>', function()
                local opts = {}
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
            end, { silent = true, desc = 'fzf proximity sort' })

            vim.keymap.set('n', '<C-b>', function()
                require 'fzf-lua'.buffers({
                    ignore_current_buffer = true,
                    fzf_opts = {
                        ["--with-nth"]     = "{-3..-2}",
                        ["--nth"]          = "-1",
                        ["--delimiter"]    = "[:\u{2002}]",
                        ["--header-lines"] = "false",
                    },
                    header = "",
                })
            end, { silent = true, desc = 'fzf buffers' })
        end
    },
}
