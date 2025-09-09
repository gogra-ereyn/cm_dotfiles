return {
	{
		{
			"kylechui/nvim-surround",
			version = "*",
			event = "VeryLazy",
			config = function()
				require("nvim-surround").setup({
				})
			end
		},
		--"tpope/vim-surround",
		"tpope/vim-fugitive",
	}
}
