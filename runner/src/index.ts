import { Container, getContainer } from "@cloudflare/containers";

export class SalamSandbox extends Container {
  override defaultPort = 8080;
  override sleepAfter = "2m";
  override enableInternet = false;
}

interface RateLimiterBinding {
  limit(options: { key: string }): Promise<{ success: boolean }>;
}

interface Env {
  SALAM_SANDBOX: DurableObjectNamespace<SalamSandbox>;
  RUNNER_RATE_LIMITER: RateLimiterBinding;
}

const MAX_CODE_BYTES = 64 * 1024;
const MAX_BODY_BYTES = MAX_CODE_BYTES + 4 * 1024;

const ALLOWED_TYPES = new Set(["program", "layout"]);
const ALLOWED_ENGINES = new Set(["interp", "llvm"]);
const ALLOWED_LANGUAGES = new Set(["en", "fa", "ar"]);

const CORS_HEADERS: Record<string, string> = {
  "Access-Control-Allow-Origin": "*",
  "Access-Control-Allow-Methods": "GET, POST, OPTIONS",
  "Access-Control-Allow-Headers": "Content-Type",
  "Access-Control-Max-Age": "86400",
};

function jsonResponse(body: unknown, status = 200): Response {
  return new Response(JSON.stringify(body), {
    status,
    headers: { "content-type": "application/json;charset=UTF-8", ...CORS_HEADERS },
  });
}

function errorResponse(status: number, error: string, message?: string, requestId?: string): Response {
  return jsonResponse({ ok: false, error, message, ...(requestId ? { requestId } : {}) }, status);
}

function errMessage(err: unknown): string {
  return err instanceof Error ? err.message : String(err);
}

type LogLevel = "info" | "warn" | "error";

function log(level: LogLevel, event: string, fields: Record<string, unknown> = {}): void {
  const line = JSON.stringify({ level, event, ts: new Date().toISOString(), ...fields });
  if (level === "error") console.error(line);
  else if (level === "warn") console.warn(line);
  else console.log(line);
}

export default {
  async fetch(request: Request, env: Env): Promise<Response> {
    const requestId = crypto.randomUUID();
    const url = new URL(request.url);

    try {
      if (request.method === "OPTIONS") {
        return new Response(null, { status: 204, headers: CORS_HEADERS });
      }

      if (url.pathname === "/" && request.method === "GET") {
        return landingPage();
      }

      if (url.pathname === "/v1/run" && request.method === "POST") {
        return await handleRun(request, env, url, requestId);
      }

      if (url.pathname === "/v1/version" && request.method === "GET") {
        return await handleVersion(env, requestId);
      }

      return errorResponse(404, "not_found", undefined, requestId);
    } catch (err) {
      log("error", "unhandled_exception", {
        requestId,
        path: url.pathname,
        method: request.method,
        error: errMessage(err),
        stack: err instanceof Error ? err.stack : undefined,
      });
      return errorResponse(500, "internal_error", "unexpected server error", requestId);
    }
  },
} satisfies ExportedHandler<Env>;

async function handleRun(request: Request, env: Env, url: URL, requestId: string): Promise<Response> {
  const clientIp = request.headers.get("cf-connecting-ip") ?? "unknown";

  const contentLength = Number(request.headers.get("content-length") ?? "0");
  if (contentLength > MAX_BODY_BYTES) {
    return errorResponse(413, "payload_too_large", `body must be under ${MAX_BODY_BYTES} bytes`, requestId);
  }

  try {
    const limitResult = await env.RUNNER_RATE_LIMITER.limit({ key: clientIp });
    if (!limitResult.success) {
      log("info", "rate_limited", { requestId, clientIp });
      return errorResponse(429, "rate_limited", "too many requests - slow down and try again shortly", requestId);
    }
  } catch (err) {
    log("warn", "rate_limiter_error", { requestId, clientIp, error: errMessage(err) });
  }

  let code: string;
  try {
    code = await request.text();
  } catch (err) {
    log("warn", "body_read_error", { requestId, clientIp, error: errMessage(err) });
    return errorResponse(400, "invalid_request", "could not read request body", requestId);
  }
  if (code.length > MAX_BODY_BYTES) {
    return errorResponse(413, "payload_too_large", `body must be under ${MAX_BODY_BYTES} bytes`, requestId);
  }

  const type = url.searchParams.get("type") ?? "program";
  const engine = url.searchParams.get("engine") ?? "interp";
  const language = url.searchParams.get("language") ?? "en";

  if (!code) {
    return errorResponse(400, "invalid_request", "request body must contain Salam source code", requestId);
  }
  if (code.length > MAX_CODE_BYTES) {
    return errorResponse(413, "payload_too_large", `code must be under ${MAX_CODE_BYTES} bytes`, requestId);
  }
  if (!ALLOWED_TYPES.has(type)) {
    return errorResponse(400, "invalid_request", "'type' query param must be 'program' or 'layout'", requestId);
  }
  if (!ALLOWED_ENGINES.has(engine)) {
    return errorResponse(400, "invalid_request", "'engine' query param must be 'interp' or 'llvm'", requestId);
  }
  if (!ALLOWED_LANGUAGES.has(language)) {
    return errorResponse(400, "invalid_request", "'language' query param must be 'en', 'fa', or 'ar'", requestId);
  }

  const sandbox = getContainer(env.SALAM_SANDBOX, requestId);

  log("info", "run_started", { requestId, clientIp, type, engine, language, codeBytes: code.length });
  const startedAt = Date.now();

  let sandboxResponse: Response;
  try {
    sandboxResponse = await sandbox.fetch("http://sandbox/run", {
      method: "POST",
      headers: { "content-type": "application/json", "x-request-id": requestId },
      body: JSON.stringify({ code, type, engine, language }),
    });
  } catch (err) {
    log("error", "sandbox_fetch_error", {
      requestId,
      clientIp,
      durationMs: Date.now() - startedAt,
      error: errMessage(err),
    });
    return errorResponse(502, "sandbox_unavailable", "the sandbox instance could not be reached", requestId);
  }

  let resultText: string;
  try {
    resultText = await sandboxResponse.text();
  } catch (err) {
    log("error", "sandbox_response_read_error", { requestId, error: errMessage(err) });
    return errorResponse(502, "sandbox_unavailable", "the sandbox response could not be read", requestId);
  }

  log("info", "run_completed", {
    requestId,
    status: sandboxResponse.status,
    durationMs: Date.now() - startedAt,
  });

  return new Response(resultText, {
    status: sandboxResponse.status,
    headers: { "content-type": "application/json;charset=UTF-8", ...CORS_HEADERS },
  });
}

async function handleVersion(env: Env, requestId: string): Promise<Response> {
  const sandbox = getContainer(env.SALAM_SANDBOX, "version-probe");
  try {
    const res = await sandbox.fetch("http://sandbox/version", {
      headers: { "x-request-id": requestId },
    });
    const text = await res.text();
    return new Response(text, {
      status: res.status,
      headers: { "content-type": "application/json;charset=UTF-8", ...CORS_HEADERS },
    });
  } catch (err) {
    log("error", "version_fetch_error", { requestId, error: errMessage(err) });
    return errorResponse(502, "sandbox_unavailable", "the sandbox instance could not be reached", requestId);
  }
}

function landingPage(): Response {
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
