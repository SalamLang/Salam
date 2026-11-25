VERSION = "1.0.0"

local micro = import("micro")
local config = import("micro/config")
local shell = import("micro/shell")
local buffer = import("micro/buffer")
local os = import("os")

local PLATFORMS = {
    "web", "js", "android", "ios", "linux", "macos", "windows", "all"
}
local ARCHES = { "x86_64", "aarch64", "i686", "arm", "riscv64" }
local MAX_CHAIN = 4
local MAX_ITEM = 28

local function contains(list, value)
    for _, v in ipairs(list) do
        if v == value then
            return true
        end
    end
    return false
end

local function quote(s)
    return '"' .. (s:gsub('"', '\\"')) .. '"'
end

local function dirname(path)
    local d = path:match("^(.*)[/\\][^/\\]*$")
    if d == nil then
        return "."
    end
    if d == "" then
        return "/"
    end
    return d
end

local function exists(path)
    local info, err = os.Stat(path)
    return err == nil and info ~= nil
end

local function project_root(path)
    local dir = dirname(path)
    local last = nil
    while dir ~= last do
        if exists(dir .. "/salam.toml") then
            return dir
        end
        last = dir
        dir = dirname(dir)
    end
    return nil
end

local function option(name)
    local ok, value = pcall(config.GetGlobalOption, "salam." .. name)
    if ok and value ~= nil then
        return value
    end
    return ""
end

local function flags()
    local out = ""
    local platform = option("platform")
    local arch = option("arch")
    if platform ~= "" then
        out = out .. " --platform=" .. platform
    end
    if arch ~= "" then
        out = out .. " --arch=" .. arch
    end
    return out
end

local function file_of(bp)
    local path = bp.Buf.AbsPath
    if path == nil or path == "" then
        micro.InfoBar():Error("Salam: save the buffer to a file first")
        return nil
    end
    if bp.Buf:Modified() then
        bp:Save()
    end
    return path
end

local function verb(bp, name, extra)
    local path = file_of(bp)
    if path == nil then
        return
    end
    local root = project_root(path)
    local cmd
    if root ~= nil then
        cmd = "cd " .. quote(root) .. " && salam " .. name .. flags() .. extra
    else
        cmd = "salam " .. name .. " " .. quote(path) .. flags() .. extra
    end
    shell.RunInteractiveShell(cmd, true, false)
end

function run(bp)
    verb(bp, "run", " --log-level=error")
end

function build(bp)
    verb(bp, "build", " --log-level=error --error-style=gcc")
end

function parse_diagnostics(text)
    local found = {}
    for line in (text .. "\n"):gmatch("(.-)\r?\n") do
        local file, row, col, kind, msg =
            line:match("^(.-):(%d+):(%d+): (%a+): (.*)$")
        if file ~= nil then
            found[#found + 1] = {
                file = file,
                row = tonumber(row),
                col = tonumber(col),
                kind = kind,
                msg = msg,
            }
        end
    end
    return found
end

function check(bp)
    local path = file_of(bp)
    if path == nil then
        return
    end
    local out = shell.ExecCommand("salam", "check", path, "--error-style=gcc")
    local found = parse_diagnostics(out or "")
    if #found == 0 then
        micro.InfoBar():Message("Salam: no errors")
        return
    end
    local first = found[1]
    bp.Cursor:GotoLoc(buffer.Loc(first.col - 1, first.row - 1))
    bp:Center()
    local more = ""
    if #found > 1 then
        more = " (+" .. (#found - 1) .. " more)"
    end
    micro.InfoBar():Error(
        "Salam " .. first.row .. ":" .. first.col .. " " .. first.msg .. more
    )
end

function format(bp)
    local path = file_of(bp)
    if path == nil then
        return
    end
    local y = bp.Cursor.Y
    local out, err = shell.ExecCommand("salam", "format", path)
    if err ~= nil then
        micro.InfoBar():Error("Salam format failed: " .. tostring(out))
        return
    end
    bp.Buf:ReOpen()
    local last = bp.Buf:LinesNum() - 1
    if y > last then
        y = last
    end
    bp.Cursor:GotoLoc(buffer.Loc(0, y))
    micro.InfoBar():Message("Salam: formatted")
end

function new_project(bp, args)
    local dir = ""
    if #args >= 1 then
        dir = " " .. quote(args[1])
    end
    shell.RunInteractiveShell("salam init" .. dir, true, false)
end

function platform(bp, args)
    if #args == 0 then
        local p = option("platform")
        local a = option("arch")
        if p == "" then
            p = "project default"
        end
        if a == "" then
            a = "host"
        end
        micro.InfoBar():Message("Salam platform: " .. p .. ", arch: " .. a)
        return
    end
    local p = args[1]
    local a = ""
    if #args >= 2 then
        a = args[2]
    end
    if p == "default" or p == "none" then
        p = ""
        a = ""
    elseif not contains(PLATFORMS, p) then
        micro.InfoBar():Error(
            "Salam: platform is one of " .. table.concat(PLATFORMS, ", ")
                .. " or default"
        )
        return
    end
    if a ~= "" and not contains(ARCHES, a) then
        micro.InfoBar():Error(
            "Salam: arch is one of " .. table.concat(ARCHES, ", ")
        )
        return
    end
    config.SetGlobalOption("salam.platform", p)
    config.SetGlobalOption("salam.arch", a)
    if p == "" then
        micro.InfoBar():Message("Salam: platform follows the project")
    else
        micro.InfoBar():Message("Salam: platform " .. p .. " " .. a)
    end
end

local function complete_platform(b)
    local ok, input = pcall(buffer.GetArg, b)
    if not ok or input == nil then
        input = ""
    end
    local out = {}
    local all = { "default" }
    for _, v in ipairs(PLATFORMS) do
        all[#all + 1] = v
    end
    for _, v in ipairs(ARCHES) do
        all[#all + 1] = v
    end
    for _, v in ipairs(all) do
        if v:sub(1, #input) == input then
            out[#out + 1] = v
        end
    end
    return out, out
end

local function strip_comment(text)
    local at = text:find("//", 1, true)
    while at ~= nil do
        local _, quotes = text:sub(1, at - 1):gsub('"', "")
        if quotes % 2 == 0 then
            return (text:sub(1, at - 1):gsub("%s+$", ""))
        end
        at = text:find("//", at + 1, true)
    end
    return text
end

local function indent_of(text)
    local lead = text:match("^[ \t]*")
    local width = 0
    for c in lead:gmatch(".") do
        if c == "\t" then
            width = width + 4
        else
            width = width + 1
        end
    end
    return width
end

local function is_end(text)
    return text == "end" or text == "پایان"
end

local function clip(text)
    local chars = {}
    for ch in text:gmatch("[%z\1-\127\194-\244][\128-\191]*") do
        chars[#chars + 1] = ch
    end
    if #chars <= MAX_ITEM then
        return text
    end
    return table.concat(chars, "", 1, MAX_ITEM - 1) .. "..."
end

function block_chain(line_at, count, y)
    local current = line_at(y) or ""
    local trimmed = strip_comment(current):gsub("^%s+", ""):gsub("%s+$", "")
    local level = indent_of(current)
    local chain = {}
    if trimmed ~= "" then
        if trimmed:sub(-1) == ":" then
            chain[1] = trimmed
        elseif is_end(trimmed) then
            level = level + 1
        end
    else
        level = 1000000
    end
    local row = y - 1
    while row >= 0 and level > 0 do
        local text = strip_comment(line_at(row) or "")
        local body = text:gsub("^%s+", ""):gsub("%s+$", "")
        if body ~= "" then
            local width = indent_of(text)
            if width < level then
                level = width
                if body:sub(-1) == ":" and not is_end(body) then
                    table.insert(chain, 1, body)
                end
            end
        end
        row = row - 1
    end
    return chain
end

function chain_text(chain)
    local items = {}
    for _, header in ipairs(chain) do
        items[#items + 1] = clip((header:gsub(":$", "")))
    end
    local first = 1
    local prefix = ""
    if #items > MAX_CHAIN then
        first = #items - MAX_CHAIN + 1
        prefix = "... > "
    end
    return prefix .. table.concat(items, " > ", first, #items)
end

function block(b)
    local y = b.Cursor.Y
    local chain = block_chain(function(n)
        return b:Line(n)
    end, b:LinesNum(), y)
    return chain_text(chain)
end

function init()
    config.RegisterCommonOption("salam", "platform", "")
    config.RegisterCommonOption("salam", "arch", "")

    config.MakeCommand("salamrun", run, config.NoComplete)
    config.MakeCommand("salambuild", build, config.NoComplete)
    config.MakeCommand("salamcheck", check, config.NoComplete)
    config.MakeCommand("salamformat", format, config.NoComplete)
    config.MakeCommand("salamnew", new_project, config.FileComplete)
    config.MakeCommand("salamplatform", platform, complete_platform)

    config.AddRuntimeFile("salam", config.RTHelp, "help/salam.md")

    micro.SetStatusInfoFn("salam.block")

    local ok, linter = pcall(import, "linter")
    if ok and linter ~= nil then
        linter.makeLinter(
            "salam",
            "salam",
            "salam",
            { "check", "%f", "--error-style=gcc" },
            "%f:%l:%c: %m",
            {},
            false,
            false,
            0,
            0
        )
    end
end
