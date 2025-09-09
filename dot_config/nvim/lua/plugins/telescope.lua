return {
    {
        'nvim-telescope/telescope.nvim',
        branch = '0.1.x',
        dependencies = {
            'nvim-lua/plenary.nvim',
           -- 'nvim-telescope/nvim-telescope-fzf-native.nvim',
            { 'nvim-tree/nvim-web-devicons',                   optional = true }
        },
        config = function()
            require('telescope').setup {

                --extensions = {
                --    fzf = {
                --        fuzzy = true,
                --        case_mode = "smart_case",
                --        override_generic_sorter = true,
                --        override_file_sorter = true,
                --    }
                --}
            }
            --require('telescope').load_extension('fzf')

            vim.keymap.set('n', '<leader>r', require('telescope.builtin').find_files, { desc = 'Find files' })
            vim.keymap.set('n', '<leader>b', require('telescope.builtin').buffers, { desc = 'Find buffers' })
        end,
    }
}
