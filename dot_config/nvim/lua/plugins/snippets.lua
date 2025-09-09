return {
    {
        "L3MON4D3/LuaSnip",
        build = "make install_jsregexp",
        version = "v2.*",
        dependencies = {
            "rafamadriz/friendly-snippets",
        },
        config = function()
            require("luasnip.loaders.from_vscode").lazy_load()
            local ls = require("luasnip")
            local snippet_path = "lua/snippets/"
            local snippet_files = vim.api.nvim_get_runtime_file(snippet_path .. "*.lua", true)

            for _, fpath in ipairs(snippet_files) do
                local filename = vim.fn.fnamemodify(fpath, ":t:r")
                local chunk = loadfile(fpath)
                if chunk then
                    local snippets = chunk()
                    if type(snippets) == "table" then
                        ls.add_snippets(filename, snippets)
                    end
                end
            end




            ---@diagnostic disable-next-line: duplicate-set-field
            vim.snippet.active = function(filter)
                filter = filter or {}
                if filter.direction == 1 then
                    return ls.expand_or_jumpable
                else
                    return ls.jumpable(filter.direction)
                end
            end

            ---@diagnostic disable-next-line: duplicate-set-field
            vim.snippet.jump = function(direction)
                if direction == 1 then
                    if ls.expandable() then
                        return ls.expand_or_jump()
                    else
                        return ls.jumpable(1) and ls.jump(1)
                    end
                else
                    return ls.jumpable(-1) and ls.jump(-1)
                end
            end

            vim.snippet.stop = ls.unlink_current
            ls.config.set_config {
                history = true,
                enable_autosnippets = false,
            }

            vim.keymap.set({ "i" }, "<C-x>", function()
                if ls.expand_or_jumpable() then
                    ls.expand_or_jump()
                end
            end, { silent = true })

            vim.api.nvim_create_user_command("ExpandSnippet", function(opts)
                local snip_name = opts.args
                local snippets = ls.get_snippets("c")
                if snippets and snippets[snip_name] then
                    ls.snip_expand(snippets[snip_name])
                else
                    print("snippet not found:", snip_name)
                    print("available snippets:", vim.inspect(snippets))
                end
            end, { nargs = 1 })


            vim.keymap.set("n", "<leader>j", function()
                require("luasnip.loaders").edit_snippet_files()
            end, { desc = "Edit snippets" })


            vim.api.nvim_create_user_command("ListSnippets", function()
                local available_snippets = {}
                for ft, snippets in pairs(ls.get_snippets()) do
                    if not available_snippets[ft] then
                        available_snippets[ft] = {}
                    end
                    for _, snippet in pairs(snippets) do
                        table.insert(available_snippets[ft], snippet.name)
                    end
                end

                --	for ft, names in pairs(available_snippets) do
                --		print("Snippets for filetype '" .. ft .. "':")
                --		for _, name in ipairs(names) do
                --			print("  - " .. name)
                --			vim.keymap.set({"i", "s"}, "<Tab>", "<Tab>", {silent = true, noremap = true})
                --			vim.keymap.set({"i", "s"}, "<S-Tab>", "<S-Tab>", {silent = true, noremap = true})
                --		end
                --		print("")
                --	end
            end, {})
            vim.keymap.set({ "n" }, "<leader>ls", ":ListSnippets<CR>")
        end

    }
}
