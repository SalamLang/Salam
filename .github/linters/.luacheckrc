std = "luajit"
cache = true

-- Neovim injects `vim` as a global, and plugins write to vim.g and friends,
-- so it has to be writable rather than read-only.
globals = { "vim" }

exclude_files = { "**/node_modules" }
