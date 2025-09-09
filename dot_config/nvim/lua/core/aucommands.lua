local function trim_whitespace()
	local save=vim.fn.winsaveview()
	vim.cmd([[keeppatterns %s/\s\+$//e]])
	vim.fn.winrestview(save)
end

local tsg=vim.api.nvim_create_augroup("TrimWhiteSpace", { clear = true })

vim.api.nvim_create_autocmd("BufWritePre", {
	group=tsg,
	callback=function()
		local excl={"markdown", "text"}
		local ct=vim.bo.filetype
		if not vim.tbl_contains(excl, ct) then
			trim_whitespace()
		end
	end,
})

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
    vim.bo.tabstop = 8
    vim.bo.shiftwidth = 8
  end
})

