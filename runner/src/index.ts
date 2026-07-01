export default {
  async fetch(request): Promise<Response> {
    const html = `<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>SALAM CORE RUNTIME</title>
    <script src="https://unpkg.com/@tailwindcss/browser@4.0.0"></script>
    <style>
        @import url('https://fonts.googleapis.com/css2?family=JetBrains+Mono:wght@300;400;500;700&display=swap');
        body {
            font-family: 'JetBrains Mono', monospace;
        }
    </style>
</head>
<body class="bg-[#0b0f17] text-slate-300 min-h-screen selection:bg-teal-500/30 selection:text-teal-200">

    <div class="h-1 w-full bg-gradient-to-r from-teal-500 via-emerald-500 to-cyan-500 shadow-[0_2px_20px_rgba(20,184,166,0.5)]"></div>

    <main class="max-w-6xl mx-auto px-4 py-12 md:py-20">

        <header class="flex flex-col md:flex-row md:items-center md:justify-between border-b border-slate-800/60 pb-8 mb-12">
            <div>
                <div class="flex items-center gap-3 mb-2">
                    <span class="text-xs font-bold tracking-widest text-teal-400 bg-teal-950/50 border border-teal-800/50 px-2.5 py-1 rounded">V4 ACTIVE</span>
                    <span class="text-xs font-bold tracking-widest text-slate-400 bg-slate-900 border border-slate-800 px-2.5 py-1 rounded">EDGE COMPILER</span>
                </div>
                <h1 class="text-3xl font-extrabold tracking-tight text-white md:text-4xl bg-gradient-to-r from-white via-slate-200 to-slate-400 bg-clip-text text-transparent">
                    SALAM_CORE <span class="text-teal-400 font-light">//</span> RUNTIME
                </h1>
            </div>
            <div class="mt-6 md:mt-0 flex items-center gap-3 bg-slate-900/80 border border-slate-800 px-5 py-3 rounded-xl backdrop-blur-md">
                <span class="relative flex h-3 w-3">
                    <span class="animate-ping absolute inline-flex h-full w-full rounded-full bg-emerald-400 opacity-75"></span>
                    <span class="relative inline-flex rounded-full h-3 w-3 bg-emerald-500"></span>
                </span>
                <span class="text-sm font-medium text-slate-200">ENGINE RUNTIME: <span class="text-emerald-400 font-bold">ONLINE</span></span>
            </div>
        </header>

        <div class="grid grid-cols-1 lg:grid-cols-12 gap-8 mb-12">

            <section class="lg:col-span-7 space-y-6">
                <h2 class="text-lg font-bold text-slate-100 tracking-wide flex items-center gap-2">
                    <span class="text-teal-500">&gt;</span> DEVELOPMENT CONTROL
                </h2>

                <div class="bg-slate-900/40 border border-slate-800/80 rounded-xl p-6 backdrop-blur-sm hover:border-slate-700/50 transition-colors duration-200">
                    <div class="flex items-center justify-between mb-4">
                        <h3 class="text-sm font-bold text-slate-200 tracking-wide">1. INITIAL PACKAGE INSTALL</h3>
                        <span class="text-xs text-slate-500">BUN ENVIRONMENT</span>
                    </div>
                    <div class="bg-[#070a0f] rounded-lg p-4 border border-slate-950 flex items-center justify-between group">
                        <code class="text-sm text-teal-400">bun install</code>
                        <span class="text-xs text-slate-600 group-hover:text-slate-400 transition-colors cursor-pointer select-none">Copy</span>
                    </div>
                </div>

                <div class="bg-slate-900/40 border border-slate-800/80 rounded-xl p-6 backdrop-blur-sm hover:border-slate-700/50 transition-colors duration-200">
                    <div class="flex items-center justify-between mb-4">
                        <h3 class="text-sm font-bold text-slate-200 tracking-wide">2. LAUNCH LOCAL DEVELOPMENT ENGINE</h3>
                        <span class="text-xs text-slate-500">WRANGLER / CLOUDFLARE</span>
                    </div>
                    <div class="bg-[#070a0f] rounded-lg p-4 border border-slate-950 flex items-center justify-between group">
                        <code class="text-sm text-emerald-400">bun run dev</code>
                        <span class="text-xs text-slate-600 group-hover:text-slate-400 transition-colors cursor-pointer select-none">Copy</span>
                    </div>
                    <p class="text-xs text-slate-500 mt-3 leading-relaxed">
                        Starts the local serverless execution stack at <span class="text-slate-400 font-medium">http://localhost:8787</span> with hot reloading active.
                    </p>
                </div>
            </section>

            <section class="lg:col-span-5 space-y-6">
                <h2 class="text-lg font-bold text-slate-100 tracking-wide flex items-center gap-2">
                    <span class="text-teal-500">&gt;</span> DEPLOYMENT & INTERFACE
                </h2>

                <div class="bg-gradient-to-br from-slate-900/80 to-slate-900/30 border border-slate-800/80 rounded-xl p-6 backdrop-blur-sm">
                    <div class="flex items-center justify-between border-b border-slate-800 pb-3 mb-3">
                        <span class="font-bold text-xs text-slate-200 tracking-wider">CI/CD AUTOMATION Pipeline</span>
                        <span class="text-[10px] font-bold text-purple-400 bg-purple-950/40 px-2 py-0.5 rounded border border-purple-900/40">GITHUB ACTIONS</span>
                    </div>
                    <p class="text-xs text-slate-400 leading-relaxed">
                        This runner is configured with <code class="text-purple-400">cloudflare/wrangler-action</code>. Push variations directly into production automatically:
                    </p>
                    <div class="bg-[#070a0f]/80 p-3 rounded border border-slate-950 mt-3 text-xs text-slate-400 font-light space-y-1">
                        <div class="flex items-center gap-2 text-emerald-400">
                            <span>✔</span> <span class="text-slate-300 font-medium">Triggers on:</span> <code>git push origin main</code>
                        </div>
                        <div class="flex items-center gap-2 text-cyan-400">
                            <span>⚡</span> <span class="text-slate-300 font-medium">Action:</span> Automated build & global edge release
                        </div>
                    </div>
                </div>

                <div class="bg-slate-900/40 border border-slate-800/80 rounded-xl p-6 backdrop-blur-sm">
                    <div class="flex items-center justify-between pb-3 mb-3 border-b border-slate-800/60">
                        <span class="font-bold text-xs text-slate-200 tracking-wider">POST PIPELINE VIA CURL</span>
                        <span class="text-[10px] font-bold text-teal-400 bg-teal-950/60 px-2 py-0.5 rounded border border-teal-900/40">JSON EXEC</span>
                    </div>
                    <pre class="bg-[#070a0f] text-slate-300 p-3.5 rounded-lg text-xs leading-5 border border-slate-950 overflow-x-auto text-left text-cyan-300">
curl -X POST https://your-runner.workers.dev/ \\
  -H "Content-Type: application/json" \\
  -d '{"code": "print(42);"}'</pre>
                </div>
            </section>
        </div>

        <section class="border-t border-slate-900 pt-10">
            <h2 class="text-sm font-bold text-slate-400 tracking-widest mb-6 uppercase">SYSTEM INFRASTRUCTURE OVERVIEW</h2>
            <div class="grid grid-cols-1 md:grid-cols-2 gap-6">

                <div class="bg-slate-950/40 border border-slate-900 rounded-xl p-6">
                    <div class="flex items-center gap-3 mb-3">
                        <div class="w-2 h-2 rounded-full bg-orange-500"></div>
                        <h3 class="text-sm font-bold text-slate-200 tracking-wide">CLOUDFLARE ENGINE</h3>
                    </div>
                    <p class="text-xs text-slate-400 leading-relaxed">
                        Runs directly on Cloudflare's serverless edge infrastructure using V8 isolates instead of standard virtual machines. This translates to near-zero cold starts, high global concurrency execution limits, and optimized low-latency request responses routed closest to users.
                    </p>
                </div>

                <div class="bg-slate-950/40 border border-slate-900 rounded-xl p-6">
                    <div class="flex items-center gap-3 mb-3">
                        <div class="w-2 h-2 rounded-full bg-cyan-500"></div>
                        <h3 class="text-sm font-bold text-slate-200 tracking-wide">WRANGLER TOOLING</h3>
                    </div>
                    <p class="text-xs text-slate-400 leading-relaxed">
                        Wrangler is the official command-line compiler and manager interface for Cloudflare Workers. It acts as the orchestration layer during development, handling system environment routing parameters, bundling sandboxed WebAssembly assets, and packaging dependencies cleanly.
                    </p>
                </div>

            </div>
        </section>

        <footer class="mt-16 pt-6 border-t border-slate-950 flex flex-col sm:flex-row items-center justify-between gap-4 text-xs text-slate-600">
            <div>DISTRIBUTED MATRIX RUNTIME SYSTEMS</div>
            <div class="flex items-center gap-4">
                <a href="#" class="hover:text-slate-400 transition-colors">CONFIG</a>
                <span>&bull;</span>
                <a href="#" class="hover:text-slate-400 transition-colors">WASM LOGS</a>
            </div>
        </footer>

    </main>

</body>`;

    return new Response(html, {
      headers: {
        "content-type": "text/html;charset=UTF-8",
      },
    });
  },
} satisfies ExportedHandler;
