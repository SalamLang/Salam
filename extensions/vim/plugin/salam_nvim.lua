if vim.g.loaded_salam_nvim == 1 then
  return
end
vim.g.loaded_salam_nvim = 1

vim.api.nvim_create_user_command('SalamCheckDiagnostics', function()
  require('salam').check()
end, { desc = 'Type-check the buffer into vim.diagnostic' })
