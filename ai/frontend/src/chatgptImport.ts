export type ZipEntry = {
  name: string;
  method: number;
  packed: number;
  size: number;
  offset: number;
  crc: number;
  flags: number;
};

export type ExportPart = {
  kind: "conversations" | "file";
  name: string;
  raw: number;
  gz: Uint8Array;
};

export const TARGET_RAW = 3 * 1024 * 1024;
export const MAX_EXTRA = 2 * 1024 * 1024;
export const MAX_TOTAL_GZ = 800 * 1024 * 1024;
export const MAX_ZIP = 8 * 1024 * 1024 * 1024;
export const MAX_RAW_ENTRY = 4 * 1024 * 1024 * 1024;
export const MAX_RATIO = 400;

export class ExportError extends Error {}

function bad(message: string): never {
  throw new ExportError(message);
}

async function bytesOf(blob: Blob): Promise<DataView> {
  return new DataView(await blob.arrayBuffer());
}

export async function readEntries(file: Blob): Promise<ZipEntry[]> {
  if (file.size < 22) bad("this file is too small to be a zip");
  if (file.size > MAX_ZIP) bad("this file is too large to import");
  const first = await bytesOf(file.slice(0, 4));
  const magic = first.getUint32(0, true);
  if (magic !== 0x04034b50 && magic !== 0x06054b50)
    bad("this is not a zip file");
  const tailLen = Math.min(file.size, 70000);
  const tail = await bytesOf(file.slice(file.size - tailLen));
  let eocd = -1;
  for (let i = tail.byteLength - 22; i >= 0; i -= 1) {
    if (tail.getUint32(i, true) === 0x06054b50) {
      eocd = i;
      break;
    }
  }
  if (eocd < 0) bad("not a zip");
  let count = tail.getUint16(eocd + 10, true);
  let size = tail.getUint32(eocd + 12, true);
  let start = tail.getUint32(eocd + 16, true);
  if (start === 0xffffffff || count === 0xffff) {
    if (eocd < 20 || tail.getUint32(eocd - 20, true) !== 0x07064b50)
      bad("broken zip64 end");
    const at = Number(tail.getBigUint64(eocd - 20 + 8, true));
    const head = await bytesOf(file.slice(at, at + 56));
    if (head.getUint32(0, true) !== 0x06064b50) bad("broken zip64 end");
    count = Number(head.getBigUint64(32, true));
    size = Number(head.getBigUint64(40, true));
    start = Number(head.getBigUint64(48, true));
  }
  if (size > 256 * 1024 * 1024) bad("the zip index is too large");
  const dir = await bytesOf(file.slice(start, start + size));
  const utf8 = new TextDecoder();
  const out: ZipEntry[] = [];
  let at = 0;
  for (let n = 0; n < count; n += 1) {
    if (at + 46 > dir.byteLength || dir.getUint32(at, true) !== 0x02014b50)
      bad("broken zip index");
    const flags = dir.getUint16(at + 8, true);
    const method = dir.getUint16(at + 10, true);
    const crc = dir.getUint32(at + 16, true);
    let packed = dir.getUint32(at + 20, true);
    let real = dir.getUint32(at + 24, true);
    const nameLen = dir.getUint16(at + 28, true);
    const extraLen = dir.getUint16(at + 30, true);
    const noteLen = dir.getUint16(at + 32, true);
    let offset = dir.getUint32(at + 42, true);
    const wanted = [
      real === 0xffffffff,
      packed === 0xffffffff,
      offset === 0xffffffff,
    ];
    const nameAt = at + 46;
    if (nameAt + nameLen + extraLen + noteLen > dir.byteLength)
      bad("broken zip index");
    const name = utf8.decode(
      new Uint8Array(dir.buffer, dir.byteOffset + nameAt, nameLen),
    );
    let x = nameAt + nameLen;
    const stop = x + extraLen;
    while (x + 4 <= stop) {
      const id = dir.getUint16(x, true);
      const len = dir.getUint16(x + 2, true);
      if (x + 4 + len > stop) break;
      if (id === 1) {
        let f = x + 4;
        const values = [real, packed, offset];
        for (let k = 0; k < 3; k += 1) {
          if (wanted[k] && f + 8 <= x + 4 + len) {
            values[k] = Number(dir.getBigUint64(f, true));
            f += 8;
          }
        }
        [real, packed, offset] = values as [number, number, number];
      }
      x += 4 + len;
    }
    if (offset + 30 > file.size || offset + packed > file.size)
      bad("the zip is truncated or damaged");
    out.push({ name, method, packed, size: real, offset, crc, flags });
    at = stop + noteLen;
  }
  return out;
}

export async function openEntry(
  file: Blob,
  entry: ZipEntry,
): Promise<ReadableStream<Uint8Array>> {
  if (entry.flags & 1) bad("this zip is password protected");
  if (entry.size > MAX_RAW_ENTRY) bad("a file in this zip is too large");
  if (entry.packed > 0 && entry.size / entry.packed > MAX_RATIO)
    bad("a file in this zip is suspiciously compressed");
  const head = await bytesOf(file.slice(entry.offset, entry.offset + 30));
  if (head.getUint32(0, true) !== 0x04034b50) bad("broken zip entry");
  const start =
    entry.offset + 30 + head.getUint16(26, true) + head.getUint16(28, true);
  const packed = file
    .slice(start, start + entry.packed)
    .stream() as ReadableStream<Uint8Array>;
  if (entry.method !== 0 && entry.method !== 8)
    bad(`unsupported zip method ${entry.method}`);
  const plain =
    entry.method === 0
      ? packed
      : packed.pipeThrough(
          new DecompressionStream(
            "deflate-raw",
          ) as unknown as ReadableWritablePair<Uint8Array, Uint8Array>,
        );
  return plain.pipeThrough(verify(entry));
}

let crcTable: Uint32Array | null = null;

export function crc32(crc: number, data: Uint8Array): number {
  if (!crcTable) {
    crcTable = new Uint32Array(256);
    for (let n = 0; n < 256; n += 1) {
      let c = n;
      for (let k = 0; k < 8; k += 1)
        c = c & 1 ? 0xedb88320 ^ (c >>> 1) : c >>> 1;
      crcTable[n] = c >>> 0;
    }
  }
  let c = ~crc >>> 0;
  for (let i = 0; i < data.length; i += 1)
    c = crcTable[(c ^ data[i]) & 0xff] ^ (c >>> 8);
  return ~c >>> 0;
}

export function verify(
  entry: ZipEntry,
): TransformStream<Uint8Array, Uint8Array> {
  let crc = 0;
  let seen = 0;
  return new TransformStream({
    transform(chunk, controller) {
      seen += chunk.length;
      if (seen > entry.size) bad(`${entry.name} is larger than the zip says`);
      crc = crc32(crc, chunk);
      controller.enqueue(chunk);
    },
    flush() {
      if (seen !== entry.size) bad(`${entry.name} is incomplete`);
      if (crc !== entry.crc)
        bad(`${entry.name} is corrupted (checksum mismatch)`);
    },
  });
}

export function checkConversation(bytes: Uint8Array): boolean {
  let value: unknown;
  try {
    value = JSON.parse(new TextDecoder("utf-8", { fatal: true }).decode(bytes));
  } catch {
    bad("a conversation in this export is not valid JSON");
  }
  const obj = value as Record<string, unknown> | null;
  if (!obj || typeof obj !== "object" || Array.isArray(obj)) return false;
  const id = obj.conversation_id ?? obj.id;
  const map = obj.mapping;
  return (
    typeof id === "string" &&
    id.length > 0 &&
    !!map &&
    typeof map === "object" &&
    !Array.isArray(map)
  );
}

export function classify(entry: ZipEntry): "conversations" | "file" | null {
  const base = (entry.name.split("/").pop() ?? "").toLowerCase();
  if (!base.endsWith(".json")) return null;
  const stem = base.slice(0, -5);
  if (stem === "conversations" || /^conversations-\d+$/.test(stem))
    return "conversations";
  const named =
    stem.startsWith("memories") ||
    stem.startsWith("memory") ||
    stem.includes("instruction") ||
    stem.startsWith("custom");
  if (named && !stem.includes("conversation") && entry.size <= MAX_EXTRA)
    return "file";
  return null;
}

function join(pieces: Uint8Array[]): Uint8Array {
  if (pieces.length === 1) return pieces[0].slice();
  let total = 0;
  for (const p of pieces) total += p.length;
  const out = new Uint8Array(total);
  let at = 0;
  for (const p of pieces) {
    out.set(p, at);
    at += p.length;
  }
  return out;
}

export async function* splitArray(
  stream: ReadableStream<Uint8Array>,
): AsyncGenerator<Uint8Array> {
  const reader = stream.getReader();
  let opened = false;
  let depth = 0;
  let quoted = false;
  let escaped = false;
  let closed = false;
  let pieces: Uint8Array[] = [];
  let begin = -1;
  try {
    for (;;) {
      const { done, value } = await reader.read();
      if (done) {
        if (closed) return;
        if (!opened) bad("conversations.json is not a JSON list");
        bad("conversations.json ends too early");
      }
      if (closed) continue;
      const n = value.length;
      if (depth > 0) begin = 0;
      for (let i = 0; i < n; i += 1) {
        const b = value[i];
        if (!opened) {
          if (b === 91) opened = true;
          continue;
        }
        if (depth === 0) {
          if (b === 123) {
            depth = 1;
            begin = i;
          } else if (b === 93) {
            closed = true;
            break;
          }
          continue;
        }
        if (quoted) {
          if (escaped) escaped = false;
          else if (b === 92) escaped = true;
          else if (b === 34) quoted = false;
          continue;
        }
        if (b === 34) quoted = true;
        else if (b === 123 || b === 91) depth += 1;
        else if (b === 125 || b === 93) {
          depth -= 1;
          if (depth === 0) {
            pieces.push(value.subarray(begin, i + 1));
            const item = join(pieces);
            pieces = [];
            begin = -1;
            yield item;
          }
        }
      }
      if (depth > 0 && begin >= 0) pieces.push(value.subarray(begin, n));
    }
  } finally {
    reader.releaseLock();
  }
}

export function pack(items: Uint8Array[]): Uint8Array {
  let total = 2 + Math.max(0, items.length - 1);
  for (const item of items) total += item.length;
  const out = new Uint8Array(total);
  out[0] = 91;
  let at = 1;
  items.forEach((item, i) => {
    if (i > 0) out[at++] = 44;
    out.set(item, at);
    at += item.length;
  });
  out[at] = 93;
  return out;
}

export async function gzip(data: Uint8Array): Promise<Uint8Array> {
  const stream = new Blob([data as BlobPart])
    .stream()
    .pipeThrough(
      new CompressionStream("gzip") as unknown as ReadableWritablePair<
        Uint8Array,
        Uint8Array
      >,
    );
  return new Uint8Array(await new Response(stream).arrayBuffer());
}

export function toBase64(bytes: Uint8Array): string {
  let text = "";
  for (let i = 0; i < bytes.length; i += 0x8000) {
    text += String.fromCharCode(...bytes.subarray(i, i + 0x8000));
  }
  return btoa(text);
}

export async function sha256Hex(text: string): Promise<string> {
  const digest = await crypto.subtle.digest(
    "SHA-256",
    new TextEncoder().encode(text),
  );
  return Array.from(new Uint8Array(digest), (b) =>
    b.toString(16).padStart(2, "0"),
  ).join("");
}

export async function fingerprint(file: {
  name: string;
  size: number;
  lastModified: number;
}): Promise<string> {
  return (
    await sha256Hex(`${file.name}:${file.size}:${file.lastModified}`)
  ).slice(0, 32);
}

export async function buildParts(
  file: Blob,
  tick: (done: number, total: number) => void = () => {},
): Promise<ExportPart[]> {
  const entries = await readEntries(file);
  const wanted = entries
    .map((entry) => ({ entry, kind: classify(entry) }))
    .filter(
      (e): e is { entry: ZipEntry; kind: "conversations" | "file" } =>
        e.kind !== null,
    )
    .sort((a, b) =>
      a.entry.name < b.entry.name ? -1 : a.entry.name > b.entry.name ? 1 : 0,
    );
  if (!wanted.some((e) => e.kind === "conversations"))
    bad("no conversations.json in this zip");
  const parts: ExportPart[] = [];
  let held = 0;
  let found = 0;
  let odd = 0;
  const keep = (part: ExportPart) => {
    held += part.gz.length;
    if (held > MAX_TOTAL_GZ) bad("this export is too large to import");
    parts.push(part);
  };
  for (let n = 0; n < wanted.length; n += 1) {
    const { entry, kind } = wanted[n];
    tick(n, wanted.length);
    const base = entry.name.split("/").pop() ?? entry.name;
    const stream = await openEntry(file, entry);
    if (kind === "file") {
      const raw = new Uint8Array(await new Response(stream).arrayBuffer());
      try {
        JSON.parse(new TextDecoder("utf-8", { fatal: true }).decode(raw));
      } catch {
        bad(`${base} is not valid JSON`);
      }
      keep({ kind, name: base, raw: raw.length, gz: await gzip(raw) });
      continue;
    }
    let group: Uint8Array[] = [];
    let size = 0;
    const flush = async () => {
      if (group.length === 0) return;
      const raw = pack(group);
      keep({
        kind: "conversations",
        name: base,
        raw: raw.length,
        gz: await gzip(raw),
      });
      group = [];
      size = 0;
    };
    for await (const item of splitArray(stream)) {
      if (checkConversation(item)) found += 1;
      else odd += 1;
      group.push(item);
      size += item.length;
      if (size >= TARGET_RAW) await flush();
    }
    await flush();
  }
  if (found === 0)
    bad(
      odd > 0
        ? "this does not look like a ChatGPT export"
        : "this export has no conversations",
    );
  tick(wanted.length, wanted.length);
  return parts;
}

export type PartBody = {
  kind: string;
  name: string;
  size: number;
  sha256: string;
  data: string;
};

export async function partBody(part: ExportPart): Promise<PartBody> {
  const data = toBase64(part.gz);
  return {
    kind: part.kind,
    name: part.name,
    size: part.gz.length,
    sha256: await sha256Hex(data),
    data,
  };
}
