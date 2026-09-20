local M = {}

function M.check()
  local health = vim.health or require('health')
  local start = health.start or health.report_start
  local ok = health.ok or health.report_ok
  local warn = health.warn or health.report_warn
  local err = health.error or health.report_error

  start('salam')

  local config = require('salam').config
  local exe = config.compiler

  if vim.fn.executable(exe) ~= 1 then
    err(('%q is not executable'):format(exe), {
      'Install the Salam compiler and put it on $PATH,',
      "or set the full path: require('salam').setup({ compiler = '/path/to/salam' })",
    })
    return
  end

  local version = vim.fn.system({ exe, 'version', '--short' })
  ok(('compiler: %s (%s)'):format(vim.fn.trim(version), vim.fn.exepath(exe)))

  if vim.fn.exists(':SalamRun') == 2 then
    ok('commands registered')
  else
    warn('the vimscript half is not loaded; check that this plugin is on your runtimepath')
  end

  if config.diagnostics then
    ok('diagnostics on save: enabled')
  else
    warn('diagnostics on save: disabled')
  end
end

return M
