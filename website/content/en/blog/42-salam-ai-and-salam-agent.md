---
title: Programming with AI in the Salam ecosystem: Salam AI, Salam Agent, the MCP server, the API and the Telegram bot
seo_title: Salam AI, Salam Agent and the Salam MCP Server Explained
description: A practical guide to Salam's AI tools: the Salam AI coding assistant, the Salam Agent desktop app, the hosted MCP server, the chat API and the Telegram bot.
keywords: Salam AI, coding assistant, Salam Agent, MCP server, AI coding agent desktop, Salam MCP, AI assistant for programming language, chat completions API, Telegram coding bot, AI agent Windows macOS Linux
category: future
summary: Salam has its own assistant, a desktop agent that works in a folder on your computer, a hosted MCP server for the AI tools you already use, an API and a Telegram bot. What each one does, how they fit together, and which limits you set.
date: 2026-11-11
image: /assets/img/blog/salam-ai-and-salam-agent-en.png
image_alt: Cover image for "Programming with AI in the Salam ecosystem: Salam AI, Salam Agent, the MCP server, the API and the Telegram bot"
---

The Salam project runs its own AI tools for programmers. **Salam AI is a coding assistant built for the Salam programming language: it answers questions about the language, writes and fixes Salam code that it checks with the Salam compiler, and looks up real standard library functions instead of inventing them.** Around it sit four other ways in: Salam Agent, a desktop app that lets the assistant work in a folder on your computer; a hosted MCP server that gives other AI tools the Salam compiler and documentation; an API for your own programs; and a Telegram bot.

This article explains what each one is for, how they relate, and what limits you control. Everything described here is free to use with a Salam AI account.

## The pieces at a glance {#overview}

| Tool                   | Where it runs                                  | Use it for                                                            |
| ---------------------- | ---------------------------------------------- | --------------------------------------------------------------------- |
| [Salam AI](/ai/)       | In the browser at ai.salamlang.ir              | Questions, writing and fixing code, explaining compiler errors        |
| [Salam Agent](/agent/) | A desktop app for Windows, macOS and Linux     | Letting the assistant run commands and edit files in a project folder |
| MCP server             | Hosted at mcp.salamlang.ir, or on your machine | Giving the AI tools you already use the Salam compiler and docs       |
| API                    | ai-api.salamlang.ir                            | Calling the assistant from your own code                              |
| Telegram bot           | @SalamAgentBot                                 | Salam AI inside Telegram                                              |

The common thread is the one described in [the future of programming languages in the AI era](/blog/programming-languages-in-the-ai-era/): a young language is poorly represented in general training data, so the useful thing an assistant can do is check its answers against the real compiler and the real library.

## Salam AI, the coding assistant {#salam-ai}

Salam's syntax differs from every mainstream language, so a general chatbot tends to fill gaps with Python or Go. Salam AI is built to check against the real thing. Its site page lists three strengths:

- **Questions about Salam.** Syntax, types, loops, structs, generics, error handling and the layout language, answered in your own words with code you can run.
- **Writing and fixing code.** Describe a program and get Salam code that the assistant checks with the Salam compiler, or paste one that does not compile and find out why.
- **The standard library.** More than 170 packages, from JSON and HTTP to databases and machine learning, with real function names and signatures looked up rather than guessed.

Code in answers is highlighted with the same grammar as the Visual Studio Code plugin. You sign up with an email address, and your chats are saved to your account. The interface is in Persian and laid out right to left, which fits the language's origins; questions and code can be in English or Persian, since both are the same language (see [one language, two scripts](/blog/one-language-two-scripts/)).

A typical use is a compiler error you do not understand. The compiler already explains a lot by itself, with codes, help lines and suggested fixes, but an assistant can connect the error to your intent. For writing Persian code in particular, see [can AI write code in Persian?](/blog/can-ai-write-persian-code/).

## Salam Agent, the desktop app {#salam-agent}

Salam Agent opens Salam AI in its own window and lets the assistant work in a folder you choose: run commands, use Docker and WSL, read and edit files and run your tests. The site describes it as Salam AI with hands. The assistant itself runs on Salam's servers; the app on your computer carries out its requests, so your files stay where they are.

Getting started takes three steps:

1. **Install and sign in.** The app registers the computer and keeps its key in the system keychain.
2. **Choose a folder.** A chat started with a work folder makes that folder the assistant's workspace: commands start there, and the file tools check every path against it and ask before using one outside it.
3. **Approve and watch.** Each command and change appears as a card in the chat. You allow it, deny it, or always allow that tool, and a running command can be stopped from its card.

What it can do is what you would do in a terminal and an editor to get a Salam project working: run commands in Bash, sh, PowerShell or cmd; build and run containers with Docker and use WSL on Windows; read, search, create and edit files; build the program, run its tests and fix what fails; and port an existing PHP, C#, C, C++ or Go project to Salam. New projects are written in Salam.

### Access levels you set per chat {#access-levels}

| Level                         | Reading in the folder | Writing and commands in the folder | Anything outside the folder      |
| ----------------------------- | --------------------- | ---------------------------------- | -------------------------------- |
| Ask before every change       | Runs                  | Asks you, or always for that tool  | Asks you                         |
| Automatic in the work folder  | Runs                  | Runs                               | Asks you                         |
| Read only                     | Runs                  | Refused                            | Asks for reads, refuses the rest |
| Accept everything, everywhere | Runs                  | Runs                               | Runs, without asking             |

A few rules apply at every level. Writes inside `.git` are always refused. Commands stop after two minutes unless the assistant asks for longer, up to 30 minutes. Only `ai.salamlang.ir` can talk to the app, and links to other sites open in your normal browser.

> [!WARNING]
> Shell commands start in the work folder but are not confined to it. In automatic mode a command can still reach paths outside the folder, so read what you ask for. The last level allows everything everywhere after a confirmation; use it only for a task and a source you trust.

### Downloads and verification {#downloads}

Builds exist for Windows 10 and 11 (64-bit installer, MSI, portable, ARM64 and 32-bit), macOS (Apple silicon and Intel, as a `.dmg` or a zipped app) and Linux (AppImage for x86-64 and ARM64, `.deb` and `.rpm`, and a 32-bit `.deb`). Every file on the release page has a `.sha256` file beside it:

```bash
sha256sum -c salam-agent-*.sha256
```

The macOS builds are not notarized yet, so the first launch needs `xattr -dr com.apple.quarantine "/Applications/Salam Agent.app"` or a right-click and Open, and Windows SmartScreen may ask for confirmation for the same reason. The app's interface is in Persian.

## The MCP server: Salam inside the AI tools you already use {#mcp-server}

The [Model Context Protocol](https://modelcontextprotocol.io) (MCP) lets AI applications call external tools. Salam's MCP server, written in Salam, gives any MCP client the Salam tools: check, run and format code, search the standard library and read the documentation. The hosted server needs nothing installed:

```json
{
  "mcpServers": {
    "salam": { "type": "http", "url": "https://mcp.salamlang.ir/mcp" }
  }
}
```

The first connection opens a browser window to sign in with your Salam AI account, on any plan including the free one. The client then keeps and refreshes its own token. Each tool call is recorded against your account and the app, so you can review what a connected app did under Connected apps on ai.salamlang.ir and disconnect it there.

The tools fall into groups:

| Group                | Tools                                                                                                                                              |
| -------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------- |
| Compiler             | `salam_check`, `salam_build`, `salam_run`, `salam_exec`, `salam_format`, `salam_translate`                                                         |
| Inspection           | `salam_inspect`, `salam_llvm_ir`, `salam_js`, `salam_version`                                                                                      |
| Language and library | `salam_docs`, `salam_stdlib_search`, `salam_stdlib_packages`, `salam_stdlib_symbols`, `salam_find_examples`, `salam_read_source`, `salam_keywords` |
| Web                  | `salam_web_search`, `salam_web_fetch`                                                                                                              |

The hosted server cannot see your files, so its compiler tools take program text instead of a path, and it never runs code: `salam_run`, `salam_exec` and `salam_build` exist only when you run the server yourself. Built locally from the repository, the server is a single native binary with no Node or Python involved, and it is read-only toward your workspace: the compiler runs in a private temporary directory and `salam_format` only reports, it never rewrites files. Three prompts, `salam-write`, `salam-port` and `salam-fix`, give a model the check-and-fix workflow and the short list of rules where Salam differs from other languages.

## The API for your own programs {#api}

The same assistant is available over HTTP in the widely used chat completions format, so existing client libraries work by pointing their base URL at `https://ai-api.salamlang.ir` and using the model name `salam`. Create a key on the keys page of Salam AI. From a terminal:

```bash
curl https://ai-api.salamlang.ir/chat/completions \
  -H "Authorization: Bearer $SALAM_AI_KEY" \
  -H "Content-Type: application/json" \
  -d '{"model":"salam","messages":[{"role":"user","content":"Write hello world in Salam"}]}'
```

And from Salam itself, with the [`net.http`](/std/net/http/) client and [`encoding.json`](/std/encoding/json/) to escape the question:

```salam
import net.http
import encoding.json
import os

func main:
    key := os.Env("SALAM_AI_KEY")
    if key.len() == 0:
        println "set SALAM_AI_KEY to a key from the keys page of Salam AI"
        ret 1
    end
    question := "Write a Salam function that reverses a list of numbers"
    body := `{"model":"salam","messages":[{"role":"user","content":"` + json.Escape(question) + `"}]}`
    mut headers := HashMap {} as HashMap<str, str>
    defer headers.Free()
    headers.Insert("Authorization", "Bearer " + key)
    headers.Insert("Content-Type", "application/json")
    res := http.PostWith("https://ai-api.salamlang.ir/chat/completions", body, headers)
    if res.err.len() > 0:
        println "request failed:", res.err
        ret 1
    end
    println "status", res.status
    println res.body
end
```

The program refuses to run without a key and prints the raw JSON reply, which you would normally parse. Keep the key in an environment variable or a secret store, never in source code.

## The Telegram bot {#telegram}

Salam AI is also available in Telegram as [@SalamAgentBot](https://t.me/SalamAgentBot). For discussion with people rather than an assistant, the Salam community has its own Telegram group, listed on the [community page](/community/) together with Discord and the other channels.

## Which one to use {#which-one}

- **Learning the language or stuck on an error:** Salam AI in the browser, next to the [course](/learn/).
- **Working on a real project folder:** Salam Agent, starting at "Ask before every change" until you trust the workflow.
- **You already use another AI coding tool:** add the MCP server, so that tool checks its Salam against the real compiler and library instead of guessing.
- **Building something on top:** the API, from any language, Salam included.

All of them sit on the tools the language provides to everyone: the compiler's diagnostics and fixes, the `SKILL.md` reference and the documented standard library. If you prefer to work without an assistant, those are on the [tools page](/tools/) and in the [developer tools article](/blog/salam-developer-tools/).

## What to keep in mind {#limits}

- **The assistant runs on Salam's servers.** What you type, and with the agent the command output and file contents it reads, is sent there to be answered.
- **Checked is not the same as correct.** Code that compiles can still do the wrong thing. Keep tests in the project and read changes before you accept them, especially at the automatic access levels.
- **The agent's shell is not a sandbox.** The folder limits apply to the file tools; commands can reach the rest of the system.
- **A young service.** These tools are new and change often, and like the language they come from a small team. We publish no accuracy figures here, because without a public evaluation they would mean little.

## Frequently asked questions {#faq}

### Is Salam AI free?

Yes. Signing up with an email address is enough, and the MCP server and Salam Agent also work with the free account.

### Does Salam Agent upload my whole project?

No. The assistant runs on Salam's servers and the app is the part on your computer that carries out its requests, so your files stay where they are. What a request produces, such as the contents of a file it reads or a command's output, is what goes back to the assistant.

### Can I use the Salam MCP server without an account?

The hosted server at mcp.salamlang.ir asks you to sign in with a Salam AI account, any plan. If you build the server from the repository and run it on your machine over stdio, there is no sign-in.

### Can Salam AI write languages other than Salam?

Salam Agent's site page says it ports existing PHP, C#, C, C++ and Go projects to Salam and writes new projects in Salam. The tools are built around the Salam compiler and library, which is where they are most useful.
