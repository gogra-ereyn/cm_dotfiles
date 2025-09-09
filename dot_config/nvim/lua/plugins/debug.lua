return {
	{
		'mfussenegger/nvim-dap',
		dependencies = {
			'williamboman/mason.nvim',
		},
		config = function()
			local dap = require('dap')
			dap.adapters.codelldb = {
				type = 'server',
				port = '${port}',
				executable = {
					command = 'codelldb',
					args = { '--port', '${port}' },
				},
			}
			dap.configurations.cpp = {
				{
					name = 'Launch file',
					type = 'codelldb',
					request = 'launch',
					program = function()
						return vim.fn.input(
						'Path to executable: ',
						vim.fn.getcwd() .. '/',
						'file'
						)
					end,
					cwd = '${workspaceFolder}',
					stopOnEntry = false,
					args = {},
				},
				{
					name = 'Attach to process',
					type = 'codelldb',
					request = 'attach',
					pid = require('dap.utils').pick_process,
					cwd = '${workspaceFolder}',
				},
			}
			dap.configurations.c = dap.configurations.cpp
		end
	},
}
