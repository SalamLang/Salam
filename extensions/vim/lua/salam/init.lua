local M = {}

M.config = {
  compiler = 'salam',
  lang = 'auto',
  run_command = 'exec',
  indent = '4',
  stdlib_path = '',
  format_on_save = false,
  diagnostics = true,
}

local namespace = vim.api.nvim_create_namespace('salam')
local group = vim.api.nvim_create_augroup('salam_nvim', { clear = true })

local PERSIAN_ONLY = '[\u{67e}\u{686}\u{698}\u{6af}\u{6a9}\u{6cc}]'

local SEVERITY = {
  error = vim.diagnostic.severity.ERROR,
  warning = vim.diagnostic.severity.WARN,
  note = vim.diagnostic.severity.HINT,
  info = vim.diagnostic.severity.INFO,
}

local function head_of(bufnr)
  return table.concat(vim.api.nvim_buf_get_lines(bufnr, 0, 200, false), '\n')
end

function M.lang(bufnr)
  if M.config.lang ~= 'auto' then
    return M.config.lang
  end

  local head = head_of(bufnr or 0)

  local code = head:match('lang%s*:%s*(%a%a)') or head:match('LANG%s*:%s*(%a%a)')
  if code then
    return code:lower()
  end

  if vim.fn.match(head, '\\v' .. PERSIAN_ONLY) >= 0 then
    return 'fa'
  end
  local arabic = select(2, head:gsub('[\216-\219][\128-\191]', ''))
  return arabic >= 3 and 'ar' or 'en'
end

local function common_args(bufnr)
  local args = { '--lang=' .. M.lang(bufnr) }
  if M.config.stdlib_path ~= '' then
    table.insert(args, '--stdlib-path=' .. M.config.stdlib_path)
  end
  return args
end

local function null_device()
  return vim.fn.has('win32') == 1 and 'NUL' or '/dev/null'
end

local function parse_json_lines(lines, bufnr)
  local name = vim.api.nvim_buf_get_name(bufnr)
  local out = {}
  for _, line in ipairs(lines) do
    if line:sub(1, 1) == '{' then
      local ok, item = pcall(vim.json.decode, line)
      if ok and type(item) == 'table' and item.line then
        local same_file = item.file == nil
          or vim.fn.fnamemodify(item.file, ':p') == vim.fn.fnamemodify(name, ':p')
        if same_file then
          table.insert(out, {
            lnum = math.max((tonumber(item.line) or 1) - 1, 0),
            col = math.max((tonumber(item.column) or 1) - 1, 0),
            severity = SEVERITY[item.level] or vim.diagnostic.severity.ERROR,
            message = item.message or '',
            code = item.code,
            source = 'salam',
          })
        end
      end
    end
  end
  return out
end

function M.check(bufnr)
  bufnr = bufnr or vim.api.nvim_get_current_buf()
  local path = vim.api.nvim_buf_get_name(bufnr)
  if path == '' then
    return
  end

  local cmd = vim.list_extend({
    M.config.compiler,
    'inspect',
    path,
    '--emit-symbol-xml',
    '--xml-out=' .. null_device(),
    '--error-format=json',
    '--log-level=error',
  }, common_args(bufnr))

  local function publish(lines)
    if vim.api.nvim_buf_is_valid(bufnr) then
      vim.diagnostic.set(namespace, bufnr, parse_json_lines(lines, bufnr))
    end
  end

  if vim.system then
    vim.system(cmd, { text = true }, function(result)
      local lines = vim.split((result.stdout or '') .. (result.stderr or ''), '\n', { trimempty = true })
      vim.schedule(function() publish(lines) end)
    end)
    return
  end

  local collected = {}
  local function collect(_, data)
    for _, line in ipairs(data or {}) do
      if line ~= '' then
        table.insert(collected, line)
      end
    end
  end
  vim.fn.jobstart(cmd, {
    stdout_buffered = true,
    stderr_buffered = true,
    on_stdout = collect,
    on_stderr = collect,
    on_exit = function() vim.schedule(function() publish(collected) end) end,
  })
end

function M.format(bufnr)
  bufnr = bufnr or vim.api.nvim_get_current_buf()
  local tmp = vim.fn.tempname() .. '.salam'
  local lines = vim.api.nvim_buf_get_lines(bufnr, 0, -1, false)
  vim.fn.writefile(lines, tmp)

  local cmd = vim.list_extend({
    M.config.compiler,
    'format',
    tmp,
    '--indent=' .. M.config.indent,
    '--error-style=gcc',
  }, common_args(bufnr))

  local output = vim.fn.system(cmd)
  if vim.v.shell_error ~= 0 then
    vim.notify('salam: formatting failed\n' .. output, vim.log.levels.ERROR)
    vim.fn.delete(tmp)
    return
  end

  local formatted = vim.fn.readfile(tmp)
  vim.fn.delete(tmp)
  if not vim.deep_equal(formatted, lines) then
    local view = vim.fn.winsaveview()
    vim.api.nvim_buf_set_lines(bufnr, 0, -1, false, formatted)
    vim.fn.winrestview(view)
  end
end

function M.setup(opts)
  M.config = vim.tbl_extend('force', M.config, opts or {})

  vim.g.salam_compiler = M.config.compiler
  vim.g.salam_lang = M.config.lang
  vim.g.salam_run_command = M.config.run_command
  vim.g.salam_format_indent = M.config.indent
  vim.g.salam_stdlib_path = M.config.stdlib_path

  vim.api.nvim_clear_autocmds({ group = group })

  if M.config.format_on_save then
    vim.api.nvim_create_autocmd('BufWritePre', {
      group = group,
      pattern = '*.salam',
      callback = function(args) M.format(args.buf) end,
    })
  end

  if M.config.diagnostics then
    vim.api.nvim_create_autocmd({ 'BufWritePost', 'BufReadPost' }, {
      group = group,
      pattern = '*.salam',
      callback = function(args) M.check(args.buf) end,
    })
    vim.api.nvim_create_autocmd('BufDelete', {
      group = group,
      pattern = '*.salam',
      callback = function(args) vim.diagnostic.reset(namespace, args.buf) end,
    })
  end
end

return M
