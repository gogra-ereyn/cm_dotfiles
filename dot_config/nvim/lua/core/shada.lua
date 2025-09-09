return {
    setup=function()
        vim.opt.exrc = true
        vim.opt.secure = true

        local workspace_path = vim.fn.getcwd()

        local cache_dir = vim.fn.stdpath("data")

        local project_name = vim.fn.fnamemodify(workspace_path, ":t")

        local project_dir = cache_dir .. "/myshada/" .. project_name

        if vim.fn.isdirectory(project_dir) == 0 then

            vim.fn.mkdir(project_dir, "p")

        end

        local shadafile = project_dir .. "/" .. vim.fn.sha256(workspace_path):sub(1, 8) .. ".shada"

        vim.opt.shadafile = shadafile

        local opts = { noremap = true, silent = true, desc = "Jump to Mark" }

        vim.keymap.set("n", "<leader>sm", function()

            require("fzf-lua").marks({ cwd = project_dir })

        end, opts)
    end
}
