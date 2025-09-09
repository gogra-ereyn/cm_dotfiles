return {
    {
  "AckslD/nvim-neoclip.lua",
  dependencies = {
     {'ibhagwan/fzf-lua'},
  },
  config = function()
    require('neoclip').setup()
  end,
  }
}
