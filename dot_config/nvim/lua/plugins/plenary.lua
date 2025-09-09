return {
    "nvim-lua/plenary.nvim",
    lazy = false,
    priority = 1000,
    config = function()
        local function find_project_root()
            local current_dir = vim.fn.expand('%:p:h')
            local root_patterns = {
                '.git',
                'package.json',
                'Cargo.toml',
                'go.mod',
                '.projectroot',
                'Makefile',
            }

            for _, pattern in ipairs(root_patterns) do
                local found = vim.fn.findfile(pattern, current_dir .. ';')
                if found ~= '' then
                    return vim.fn.fnamemodify(found, ':h')
                end

                local found_dir = vim.fn.finddir(pattern, current_dir .. ';')
                if found_dir ~= '' then
                    return vim.fn.fnamemodify(found_dir, ':h')
                end
            end

            return nil
        end

        local function setup_project_shada()
            local root = find_project_root()

            if root then
                local project_name = vim.fn.fnamemodify(root, ':t')
                local safe_path = string.gsub(root, '[/\\:]', '_')
                local shada_path = vim.fn.stdpath('state') .. '/shada/project_' .. safe_path .. '.shada'

                local shada_dir = vim.fn.fnamemodify(shada_path, ':h')
                if vim.fn.isdirectory(shada_dir) == 0 then
                    vim.fn.mkdir(shada_dir, 'p')
                end

                vim.opt.shadafile = shada_path
                vim.cmd('rshada!')

                vim.notify("Using project shada for: " .. project_name, vim.log.levels.INFO)
            end
        end

        --vim.api.nvim_create_autocmd("VimEnter", {
        --    callback = setup_project_shada,
        --})
    end,
}
