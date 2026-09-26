// Cloudflare Worker that runs a Salam program (compiled to wasm32-wasi) once
// per request, CGI style: the request arrives as environment variables plus
// stdin, and the program answers on stdout with "Status:"/header lines, a blank
// line, and the body. Routing lives entirely in main.salam.
import salamModule from "./build/app.wasm";

const ERRNO_SUCCESS = 0;
const ERRNO_BADF = 8;
const ERRNO_NOSYS = 52;
const ERRNO_SPIPE = 70;

class ProcExit extends Error {
  constructor(code) {
    super(`proc_exit(${code})`);
    this.code = code;
  }
}

const encoder = new TextEncoder();

function encodeStrings(list) {
  return list.map((s) => encoder.encode(`${s}\0`));
}

// Minimal WASI preview1 host: stdio, args, env, clocks, random. No filesystem.
function createWasi({ args, env, stdin }) {
  const argv = encodeStrings(args);
  const envp = encodeStrings(Object.entries(env).map(([k, v]) => `${k}=${v}`));
  const stdout = [];
  const stderr = [];
  let stdinOffset = 0;
  let memory;

  const view = () => new DataView(memory.buffer);
  const bytes = () => new Uint8Array(memory.buffer);

  function writeStringTable(list, ptrsPtr, bufPtr) {
    const dv = view();
    const mem = bytes();
    let offset = bufPtr;
    list.forEach((s, i) => {
      dv.setUint32(ptrsPtr + i * 4, offset, true);
      mem.set(s, offset);
      offset += s.length;
    });
    return ERRNO_SUCCESS;
  }

  function sizes(list, countPtr, sizePtr) {
    const dv = view();
    dv.setUint32(countPtr, list.length, true);
    dv.setUint32(
      sizePtr,
      list.reduce((n, s) => n + s.length, 0),
      true,
    );
    return ERRNO_SUCCESS;
  }

  const impl = {
    args_sizes_get: (c, s) => sizes(argv, c, s),
    args_get: (p, b) => writeStringTable(argv, p, b),
    environ_sizes_get: (c, s) => sizes(envp, c, s),
    environ_get: (p, b) => writeStringTable(envp, p, b),

    clock_time_get(_id, _precision, timePtr) {
      view().setBigUint64(timePtr, BigInt(Date.now()) * 1_000_000n, true);
      return ERRNO_SUCCESS;
    },
    clock_res_get(_id, resPtr) {
      view().setBigUint64(resPtr, 1_000_000n, true);
      return ERRNO_SUCCESS;
    },
    random_get(ptr, len) {
      crypto.getRandomValues(new Uint8Array(memory.buffer, ptr, len));
      return ERRNO_SUCCESS;
    },

    fd_write(fd, iovs, iovsLen, nwrittenPtr) {
      if (fd !== 1 && fd !== 2) return ERRNO_BADF;
      const dv = view();
      let written = 0;
      for (let i = 0; i < iovsLen; i++) {
        const ptr = dv.getUint32(iovs + i * 8, true);
        const len = dv.getUint32(iovs + i * 8 + 4, true);
        (fd === 1 ? stdout : stderr).push(bytes().slice(ptr, ptr + len));
        written += len;
      }
      dv.setUint32(nwrittenPtr, written, true);
      return ERRNO_SUCCESS;
    },
    fd_read(fd, iovs, iovsLen, nreadPtr) {
      if (fd !== 0) return ERRNO_BADF;
      const dv = view();
      let read = 0;
      for (let i = 0; i < iovsLen && stdinOffset < stdin.length; i++) {
        const ptr = dv.getUint32(iovs + i * 8, true);
        const len = dv.getUint32(iovs + i * 8 + 4, true);
        const chunk = stdin.subarray(stdinOffset, stdinOffset + len);
        bytes().set(chunk, ptr);
        stdinOffset += chunk.length;
        read += chunk.length;
      }
      dv.setUint32(nreadPtr, read, true);
      return ERRNO_SUCCESS;
    },
    fd_fdstat_get(fd, statPtr) {
      if (fd > 2) return ERRNO_BADF;
      const dv = view();
      dv.setUint8(statPtr, 2); // filetype: character device
      dv.setUint16(statPtr + 2, 0, true);
      dv.setBigUint64(statPtr + 8, 0n, true);
      dv.setBigUint64(statPtr + 16, 0n, true);
      return ERRNO_SUCCESS;
    },
    fd_seek: () => ERRNO_SPIPE,
    fd_close: () => ERRNO_SUCCESS,
    fd_prestat_get: () => ERRNO_BADF,
    proc_exit(code) {
      throw new ProcExit(code);
    },
  };

  // Anything the module imports that we do not implement reports ENOSYS
  // instead of failing instantiation.
  const wasiImports = {};
  for (const imp of WebAssembly.Module.imports(salamModule)) {
    if (imp.module !== "wasi_snapshot_preview1" || imp.kind !== "function")
      continue;
    wasiImports[imp.name] = impl[imp.name] ?? (() => ERRNO_NOSYS);
  }

  return {
    imports: { wasi_snapshot_preview1: wasiImports },
    bind(instance) {
      memory = instance.exports.memory;
    },
    stdout: () => concat(stdout),
    stderr: () => new TextDecoder().decode(concat(stderr)),
  };
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

function indexOfBlankLine(buf) {
  for (let i = 0; i + 1 < buf.length; i++) {
    if (buf[i] === 10 && buf[i + 1] === 10) return i;
  }
  return -1;
}

// Parse a CGI response: header lines, a blank line, then the raw body.
function toResponse(out, method) {
  const split = indexOfBlankLine(out);
  if (split < 0) {
    return new Response("salam program produced no CGI headers\n", {
      status: 502,
    });
  }
  const headers = new Headers();
  let status = 200;
  for (const line of new TextDecoder()
    .decode(out.subarray(0, split))
    .split("\n")) {
    const colon = line.indexOf(":");
    if (colon <= 0) continue;
    const name = line.slice(0, colon).trim();
    const value = line.slice(colon + 1).trim();
    if (name.toLowerCase() === "status")
      status = Number.parseInt(value, 10) || 500;
    else headers.append(name, value);
  }
  const body = method === "HEAD" ? null : out.subarray(split + 2);
  return new Response(body, { status, headers });
}

export default {
  async fetch(request) {
    const url = new URL(request.url);
    const stdin = new Uint8Array(await request.arrayBuffer());
    const wasi = createWasi({
      args: ["salam-app"],
      env: {
        GATEWAY_INTERFACE: "CGI/1.1",
        REQUEST_METHOD: request.method,
        PATH_INFO: decodeURIComponent(url.pathname),
        QUERY_STRING: url.search.slice(1),
        CONTENT_LENGTH: String(stdin.length),
        CONTENT_TYPE: request.headers.get("content-type") ?? "",
        SERVER_NAME: url.hostname,
        REMOTE_ADDR: request.headers.get("cf-connecting-ip") ?? "",
      },
      stdin,
    });

    const instance = await WebAssembly.instantiate(salamModule, wasi.imports);
    wasi.bind(instance);
    try {
      instance.exports._start();
    } catch (err) {
      if (!(err instanceof ProcExit)) {
        console.error("salam program crashed:", err, wasi.stderr());
        return new Response("internal error\n", { status: 500 });
      }
      if (err.code !== 0) {
        console.error(`salam program exited with ${err.code}:`, wasi.stderr());
        return new Response("internal error\n", { status: 500 });
      }
    }
    return toResponse(wasi.stdout(), request.method);
  },
};
