export function landingPage(): Response {
  const html = `<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>SALAM CORE RUNTIME</title>
    <script src="https://unpkg.com/@tailwindcss/browser@4.0.0"></script>
    <style>
        @import url('https://fonts.googleapis.com/css2?family=JetBrains+Mono:wght@300;400;500;700&display=swap');
        body { font-family: 'JetBrains Mono', monospace; }
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

            <section class="lg:col-span-6 space-y-6">
                <h2 class="text-lg font-bold text-slate-100 tracking-wide flex items-center gap-2">
                    <span class="text-teal-500">&gt;</span> RUN SALAM CODE
                </h2>

                <div class="bg-slate-900/40 border border-slate-800/80 rounded-xl p-6 backdrop-blur-sm">
                    <div class="flex items-center justify-between pb-3 mb-3 border-b border-slate-800/60">
                        <span class="font-bold text-xs text-slate-200 tracking-wider">POST /v1/run</span>
                        <span class="text-[10px] font-bold text-teal-400 bg-teal-950/60 px-2 py-0.5 rounded border border-teal-900/40">PLAIN TEXT</span>
                    </div>
                    <pre class="bg-[#070a0f] text-cyan-300 p-3.5 rounded-lg text-xs leading-5 border border-slate-950 overflow-x-auto text-left">curl -X POST "https://runner.salamlang.workers.dev/v1/run?type=program&engine=interp&language=en" \\
  -H "Content-Type: text/plain" \\
  --data-binary @- <<'SALAM'
func main:
  println "hi"
end
SALAM</pre>
                    <p class="text-xs text-slate-500 mt-3 leading-relaxed">
                        Request body is the raw Salam source code. All other options are query params:<br>
                        <span class="text-slate-400">type</span>: program | layout &middot;
                        <span class="text-slate-400">engine</span>: interp | llvm &middot;
                        <span class="text-slate-400">language</span>: en | fa | ar
                    </p>
                </div>

                <div class="bg-slate-900/40 border border-slate-800/80 rounded-xl p-6 backdrop-blur-sm">
                    <div class="flex items-center justify-between pb-3 mb-3 border-b border-slate-800/60">
                        <span class="font-bold text-xs text-slate-200 tracking-wider">GET /v1/version</span>
                    </div>
                    <p class="text-xs text-slate-400 leading-relaxed">Returns the pinned Salam version baked into the current sandbox image.</p>
                </div>
            </section>

            <section class="lg:col-span-6 space-y-6">
                <h2 class="text-lg font-bold text-slate-100 tracking-wide flex items-center gap-2">
                    <span class="text-teal-500">&gt;</span> SANDBOX MODEL
                </h2>
                <div class="bg-slate-950/40 border border-slate-900 rounded-xl p-6 space-y-3 text-xs text-slate-400 leading-relaxed">
                    <p>Every request is routed to a <span class="text-slate-200">fresh Cloudflare Containers instance</span> keyed by a per-request id - concurrent requests never share a filesystem, so there is nothing to collide over.</p>
                    <p>Each run is wall-clock timed out, output-capped, and executed as a non-root user inside an image with <span class="text-slate-200">no shell and no network tooling</span>.</p>
                    <p>Free and open (CORS: *), protected by per-IP rate limiting.</p>
                </div>
            </section>
        </div>

        <footer class="mt-16 pt-6 border-t border-slate-950 flex flex-col sm:flex-row items-center justify-between gap-4 text-xs text-slate-600">
            <div>DISTRIBUTED MATRIX RUNTIME SYSTEMS</div>
            <div class="flex items-center gap-4">
                <a href="https://github.com/SalamLang/Salam" class="hover:text-slate-400 transition-colors">SOURCE</a>
                <span>&bull;</span>
                <a href="/v1/version" class="hover:text-slate-400 transition-colors">VERSION</a>
            </div>
        </footer>

    </main>

</body>
</html>`;

  return new Response(html, {
    headers: { "content-type": "text/html;charset=UTF-8" },
  });
}
