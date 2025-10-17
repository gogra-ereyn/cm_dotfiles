return {
    {
        "neovim/nvim-lspconfig",
        dependencies = {
            "folke/neodev.nvim",
            { "j-hui/fidget.nvim",   opts = {} },
            { 'mrcjkb/rustaceanvim', version = '^5', lazy = false },
            "stevearc/conform.nvim",
            "b0o/SchemaStore.nvim",

        },
        config = function()
            require("neodev").setup {}

            local capabilities = nil
            if pcall(require, "cmp_nvim_lsp") then
                capabilities = require("cmp_nvim_lsp").default_capabilities()
            end

            local lspconfig = require "lspconfig"
            local servers = {
                bashls = true,
                lua_ls = true,
                jsonls = {
                    settings = {
                        json = {
                            schemas = require("schemastore").json.schemas(),
                            validate = { enable = true },
                        },
                    },
                },
                ruby_lsp = {
                    cmd = { "ruby-lsp" },
                    filetypes = { "ruby" },
                    init_options = { formatter = "auto", linters = { "rubocop" } },
                },
                yamlls = {
                    settings = {
                        yaml = {
                            schemaStore = {
                                enable = false,
                                url = "",
                            },
                            schemas = require("schemastore").yaml.schemas(),
                        },
                    },
                },
                marksman = {
                    filetypes = { "markdown", "markdown.mdx" },
                    root_dir = lspconfig.util.root_pattern(".git", ".marksman.toml"),
                },
                clangd = {
                    cmd = { "clangd", "--background-index", "--enable-config", "--clang-tidy", "--cross-file-rename", "--completion-style=detailed", "--query-driver=**", "--header-insertion=iwyu" },
                    filetypes = { "c", "cpp" },
                    init_options = {
                        clangdFileStatus = true,
                        usePlaceholders = true,
                        completeUnimported = true,
                        semanticHighlighting = true,
                    },
                    root_dir = function(fname)
                        return lspconfig.util.root_pattern("compile_commands.json",
                                "compile_flags.txt", ".git")(fname) or
                            vim.fn.getcwd()
                    end,
                },
            }

            for name, config in pairs(servers) do
                if config == true then
                    config = {}
                end
                config = vim.tbl_deep_extend("force", {}, {
                    capabilities = capabilities,
                }, config)

                lspconfig[name].setup(config)
            end


            vim.api.nvim_create_autocmd('LspAttach', {
                desc = 'lspa',
                callback = function(ev)
                    vim.bo[ev.buf].omnifunc = 'v:lua.vim.lsp.omnifunc'
                    local opts = { buffer = ev.buf }
                    vim.keymap.set('n', 'gD', vim.lsp.buf.declaration, opts)
                    vim.keymap.set('n', 'gd', vim.lsp.buf.definition, opts)
                    vim.keymap.set('n', 'K', vim.lsp.buf.hover, opts)
                    vim.keymap.set('n', 'gi', vim.lsp.buf.implementation, opts)
                    vim.keymap.set('n', '<sK>', vim.lsp.buf.signature_help, opts)
                    vim.keymap.set('n', '<leader>D', vim.lsp.buf.type_definition, opts)
                    vim.keymap.set('n', '<space>rn', vim.lsp.buf.rename, opts)
                    vim.keymap.set('n', '<leader>f', function()
                        vim.lsp.buf.format { async = true }
                    end, opts)
                    vim.keymap.set('n', '<leader>e', vim.diagnostic.open_float)
                    vim.keymap.set({ 'n', 'v' }, '<leader>a', vim.lsp.buf.code_action, opts)
                end
            })
        end,
    },
}
