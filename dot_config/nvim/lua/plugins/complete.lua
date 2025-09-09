return {
	{
		"hrsh7th/nvim-cmp",
		commit = "8c82d0bd31299dbff7f8e780f5e06d2283de9678",
		pin=true,
		lazy = false,
		priority = 100,
		dependencies = {
			"onsails/lspkind.nvim",
			"hrsh7th/cmp-nvim-lsp",
			"hrsh7th/cmp-path",
			"hrsh7th/cmp-buffer",
		},
		config = function()
			vim.opt.completeopt = { "menu", "menuone", "noselect" }
			vim.opt.shortmess:append "c"
			local lspkind = require "lspkind"
			lspkind.init {}
			local cmp = require "cmp"
			cmp.setup {
				sources = {
					{ name = "nvim_lsp" },
					{ name = "path" },
					--{ name = "buffer" },
				},
				experimental = {
					ghost_text=true,
				},
				mapping = {
					["<Down>"] = cmp.mapping.select_next_item { behavior = cmp.SelectBehavior.Insert },
					["<Up>"] = cmp.mapping.select_prev_item { behavior = cmp.SelectBehavior.Insert },
					["<CR>"] = cmp.mapping(
					cmp.mapping.confirm {
						--behavior = cmp.ConfirmBehavior.Insert,
						select = false,
					},
					{ "i", "c" }
					),
				},
			}

			cmp.setup.cmdline(':', {
				sources = cmp.config.sources({
					{ name = 'path' }
				})
			})


			vim.diagnostic.config({
				virtual_text = true,
				signs = true,
				update_in_insert = true,
				underline = true,
				severity_sort = false,
				float = true,
			})
		end,
	},
	{
		"ray-x/lsp_signature.nvim",
		event = "VeryLazy",
		opts = {},
		config = function(_, opts)
			require "lsp_signature".setup({
				doc_lines = 0,
				handler_opts = {
					border = "none"
				},
			})
		end
	},
}
