// Salam net/http on Cloudflare Workers.
//
// A Salam program that serves HTTP with `net/http` + `net/router` is compiled
// to WebAssembly with `salam build --target=wasm32-wasi`. On that target,
// `http.Run` (and RunBackground / RunTLS) serves exactly one request per
// process instead of opening a socket:
//
//   stdin (fd 0)   the raw HTTP/1.1 request   (written by this runtime)
//   fd 3           the raw HTTP/1.1 response  (read by this runtime)
//   stdout / err   program output, forwarded to the Worker's logs
//
// Usage from a Worker entry point:
//
//   import app from "./build/app.wasm";
//   import { createWorker } from "<path to std>/net/http/cloudflare/worker.js";
//   export default createWorker(app);
//
// String bindings (wrangler `[vars]` and secrets) are exposed to the program as
// environment variables, so `os.Env("NAME")` works inside Salam.

const ERRNO_SUCCESS = 0;
const ERRNO_BADF = 8;
const ERRNO_NOSYS = 52;
const ERRNO_SPIPE = 70;

const FD_STDIN = 0;
const FD_STDOUT = 1;
const FD_STDERR = 2;
const FD_RESPONSE = 3;

const HOP_BY_HOP = new Set([
  "connection",
  "keep-alive",
  "transfer-encoding",
  "content-length",
  "upgrade",
]);

const encoder = new TextEncoder();
const decoder = new TextDecoder();

class ProcExit extends Error {
  constructor(code) {
    super(`proc_exit(${code})`);
    this.code = code;
  }
}

function concat(chunks) {
  const out = new Uint8Array(chunks.reduce((n, c) => n + c.length, 0));
  let offset = 0;
  for (const c of chunks) {
    out.set(c, offset);
    offset += c.length;
  }
  return out;
}

function cString(s) {
  return encoder.encode(`${s}\0`);
}

// Line-buffered sink for the program's stdout/stderr.
function logSink(emit) {
  let pending = "";
  return {
    write(bytes) {
      pending += decoder.decode(bytes, { stream: true });
      let nl = pending.indexOf("\n");
      while (nl >= 0) {
        emit(pending.slice(0, nl));
        pending = pending.slice(nl + 1);
        nl = pending.indexOf("\n");
      }
    },
    flush() {
      if (pending.length > 0) emit(pending);
      pending = "";
    },
  };
}

// Minimal WASI preview1 host: stdio, args, env, clocks, random. No filesystem.
function createWasi(module, { args, env, stdin }) {
  const argv = args.map(cString);
  const envp = Object.entries(env).map(([k, v]) => cString(`${k}=${v}`));
  const response = [];
  const stdout = logSink((line) => console.log(line));
  const stderr = logSink((line) => console.error(line));
  let stdinOffset = 0;
  let memory;

  const view = () => new DataView(memory.buffer);
  const bytes = () => new Uint8Array(memory.buffer);

  function writeTable(list, ptrs, buf) {
    const dv = view();
    let offset = buf;
    list.forEach((s, i) => {
      dv.setUint32(ptrs + i * 4, offset, true);
      bytes().set(s, offset);
      offset += s.length;
    });
    return ERRNO_SUCCESS;
  }

  function tableSizes(list, countPtr, sizePtr) {
    const dv = view();
    dv.setUint32(countPtr, list.length, true);
    dv.setUint32(
      sizePtr,
      list.reduce((n, s) => n + s.length, 0),
      true,
    );
    return ERRNO_SUCCESS;
  }

  function iovecs(iovs, iovsLen, fn) {
    const dv = view();
    let total = 0;
    for (let i = 0; i < iovsLen; i++) {
      const ptr = dv.getUint32(iovs + i * 8, true);
      const len = dv.getUint32(iovs + i * 8 + 4, true);
      const n = fn(ptr, len);
      total += n;
      if (n < len) break;
    }
    return total;
  }

  const impl = {
    args_sizes_get: (c, s) => tableSizes(argv, c, s),
    args_get: (p, b) => writeTable(argv, p, b),
    environ_sizes_get: (c, s) => tableSizes(envp, c, s),
    environ_get: (p, b) => writeTable(envp, p, b),

    clock_time_get(_id, _precision, out) {
      view().setBigUint64(out, BigInt(Date.now()) * 1_000_000n, true);
      return ERRNO_SUCCESS;
    },
    clock_res_get(_id, out) {
      view().setBigUint64(out, 1_000_000n, true);
      return ERRNO_SUCCESS;
    },
    random_get(ptr, len) {
      crypto.getRandomValues(new Uint8Array(memory.buffer, ptr, len));
      return ERRNO_SUCCESS;
    },
    sched_yield: () => ERRNO_SUCCESS,

    fd_write(fd, iovs, iovsLen, nwrittenPtr) {
      let sink;
      if (fd === FD_RESPONSE) sink = (b) => response.push(b);
      else if (fd === FD_STDOUT) sink = (b) => stdout.write(b);
      else if (fd === FD_STDERR) sink = (b) => stderr.write(b);
      else return ERRNO_BADF;
      const n = iovecs(iovs, iovsLen, (ptr, len) => {
        sink(bytes().slice(ptr, ptr + len));
        return len;
      });
      view().setUint32(nwrittenPtr, n, true);
      return ERRNO_SUCCESS;
    },
    fd_read(fd, iovs, iovsLen, nreadPtr) {
      if (fd !== FD_STDIN) return ERRNO_BADF;
      const n = iovecs(iovs, iovsLen, (ptr, len) => {
        const chunk = stdin.subarray(stdinOffset, stdinOffset + len);
        bytes().set(chunk, ptr);
        stdinOffset += chunk.length;
        return chunk.length;
      });
      view().setUint32(nreadPtr, n, true);
      return ERRNO_SUCCESS;
    },
    fd_fdstat_get(fd, out) {
      if (fd > FD_RESPONSE) return ERRNO_BADF;
      const dv = view();
      dv.setUint8(out, 2); // character device
      dv.setUint16(out + 2, 0, true);
      dv.setBigUint64(out + 8, 0n, true);
      dv.setBigUint64(out + 16, 0n, true);
      return ERRNO_SUCCESS;
    },
    fd_seek: () => ERRNO_SPIPE,
    fd_close: () => ERRNO_SUCCESS,
    fd_prestat_get: () => ERRNO_BADF,
    proc_exit(code) {
      throw new ProcExit(code);
    },
  };

  const wasi = {};
  for (const imp of WebAssembly.Module.imports(module)) {
    if (imp.module !== "wasi_snapshot_preview1" || imp.kind !== "function")
      continue;
    wasi[imp.name] = impl[imp.name] ?? (() => ERRNO_NOSYS);
  }

  return {
    imports: { wasi_snapshot_preview1: wasi },
    bind(instance) {
      memory = instance.exports.memory;
    },
    response: () => concat(response),
    flushLogs() {
      stdout.flush();
      stderr.flush();
    },
  };
}

async function encodeRequest(request) {
  const url = new URL(request.url);
  const body = new Uint8Array(await request.arrayBuffer());
  let head = `${request.method} ${url.pathname}${url.search} HTTP/1.1\r\n`;
  if (!request.headers.has("host")) head += `Host: ${url.host}\r\n`;
  for (const [name, value] of request.headers) {
    if (HOP_BY_HOP.has(name)) continue;
    head += `${name}: ${value.replace(/[\r\n]/g, " ")}\r\n`;
  }
  head += `Content-Length: ${body.length}\r\nConnection: close\r\n\r\n`;
  return concat([encoder.encode(head), body]);
}

function indexOfCrlfCrlf(buf) {
  for (let i = 0; i + 3 < buf.length; i++) {
    if (
      buf[i] === 13 &&
      buf[i + 1] === 10 &&
      buf[i + 2] === 13 &&
      buf[i + 3] === 10
    )
      return i;
  }
  return -1;
}

function decodeResponse(raw, method) {
  const split = indexOfCrlfCrlf(raw);
  if (split < 0) return null;
  const lines = decoder.decode(raw.subarray(0, split)).split("\r\n");
  const status = Number.parseInt(lines[0].split(" ")[1], 10);
  if (!Number.isInteger(status)) return null;
  const headers = new Headers();
  let length = -1;
  for (const line of lines.slice(1)) {
    const colon = line.indexOf(":");
    if (colon <= 0) continue;
    const name = line.slice(0, colon).trim();
    const value = line.slice(colon + 1).trim();
    const lower = name.toLowerCase();
    if (lower === "content-length") length = Number.parseInt(value, 10);
    if (HOP_BY_HOP.has(lower)) continue;
    headers.append(name, value);
  }
  let body = raw.subarray(split + 4);
  if (length >= 0 && length < body.length) body = body.subarray(0, length);
  const empty =
    method === "HEAD" ||
    status === 204 ||
    status === 304 ||
    (status >= 100 && status < 200);
  return new Response(empty ? null : body, { status, headers });
}

function stringBindings(env) {
  const out = {};
  for (const [k, v] of Object.entries(env ?? {})) {
    if (typeof v === "string") out[k] = v;
  }
  return out;
}

export function createWorker(module, { args = ["salam"] } = {}) {
  return {
    async fetch(request, env) {
      const wasi = createWasi(module, {
        args,
        env: { ...stringBindings(env), SALAM_EDGE: "cloudflare" },
        stdin: await encodeRequest(request),
      });
      const instance = await WebAssembly.instantiate(module, wasi.imports);
      wasi.bind(instance);
      let exitCode = 0;
      try {
        instance.exports._start();
      } catch (err) {
        if (!(err instanceof ProcExit)) {
          wasi.flushLogs();
          console.error("salam: program trapped:", err);
          return new Response("Internal Server Error\n", { status: 500 });
        }
        exitCode = err.code;
      }
      wasi.flushLogs();
      const response = decodeResponse(wasi.response(), request.method);
      if (response) return response;
      console.error(
        `salam: program exited (${exitCode}) without answering; is http.Run reached for ${request.method} ${new URL(request.url).pathname}?`,
      );
      return new Response("Bad Gateway\n", { status: 502 });
    },
  };
}
