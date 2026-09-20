-- Neovim-only bootstrap. Vim never reads plugin/*.lua, so the vimscript
-- half stays the only thing that loads there.
if vim.g.loaded_salam_nvim == 1 then
  return
end
vim.g.loaded_salam_nvim = 1

vim.api.nvim_create_user_command('SalamCheckDiagnostics', function()
  require('salam').check()
end, { desc = 'Type-check the buffer into vim.diagnostic' })
