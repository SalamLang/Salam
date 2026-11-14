import { useEffect, useState } from "react";
import { api, type Capabilities, type FileMeta, type OutgoingFile } from "./api";
import { IconClose, IconDownload, IconFile, IconFolder } from "./icons";
import { Lightbox, downloadUrl } from "./Lightbox";
import { pushToast } from "./Toasts";
import { toPersianDigits } from "./theme";
import { t } from "./i18n";

export type Pending = { key: string; file: File; kind: FileMeta["kind"]; preview?: string };

const IMAGE_EXTS = ["png", "jpg", "jpeg", "gif", "webp"];

let capsPromise: Promise<Capabilities | null> | null = null;

export function useCapabilities(): Capabilities | null {
  const [caps, setCaps] = useState<Capabilities | null>(null);
  useEffect(() => {
    let live = true;
    if (!capsPromise) capsPromise = api.capabilities().catch(() => null);
    capsPromise.then((c) => {
      if (!c) capsPromise = null;
      if (live) setCaps(c);
    });
    return () => {
      live = false;
    };
  }, []);
  return caps;
}

function extOf(name: string): string {
  const base = name.split(/[\\/]/).pop()?.toLowerCase() ?? "";
  const dot = base.lastIndexOf(".");
  return dot < 0 ? "" : base.slice(dot + 1);
}

const MIME_IMAGE: Record<string, string> = {
  "image/png": "png",
  "image/jpeg": "jpg",
  "image/gif": "gif",
  "image/webp": "webp",
};

export function fmtSize(n: number): string {
  if (n < 1024) return t(`${toPersianDigits(n)} بایت`, `${n} bytes`);
  if (n < 1024 * 1024) return t(`${toPersianDigits(Math.round(n / 1024))} کیلوبایت`, `${Math.round(n / 1024)} KB`);
  return t(`${toPersianDigits((n / 1024 / 1024).toFixed(1))} مگابایت`, `${(n / 1024 / 1024).toFixed(1)} MB`);
}

export function acceptList(caps: Capabilities): string {
  const exts = [...caps.text, "zip"];
  if (caps.image) exts.push(...IMAGE_EXTS);
  if (caps.pdf) exts.push("pdf");
  return exts.map((e) => `.${e}`).join(",");
}

export function kindOf(file: File, caps: Capabilities): { kind?: FileMeta["kind"]; problem?: string } {
  const ext = extOf(file.name);
  const base = file.name.split(/[\\/]/).pop()?.toLowerCase() ?? "";
  const mime = (file.type || "").toLowerCase();
  if (IMAGE_EXTS.includes(ext) || mime in MIME_IMAGE) {
    if (!caps.image) return { problem: t("مدل فعلی دستیار تصویر نمی‌پذیرد.", "The current assistant model does not accept images.") };
    if (file.size > caps.max_image) return { problem: t(`تصویر «${file.name || t("الحاقی", "clipboard")}» بزرگ‌تر از ${fmtSize(caps.max_image)} است.`, `The image "${file.name || "clipboard"}" is larger than ${fmtSize(caps.max_image)}.`) };
    return { kind: "image" };
  }
  if (ext === "pdf" || mime === "application/pdf") {
    if (!caps.pdf) return { problem: t("مدل فعلی دستیار فایل PDF نمی‌پذیرد. متن آن را در یک فایل متنی بفرستید.", "The current assistant model does not accept PDF files. Send its text in a text file instead.") };
    if (file.size > caps.max_pdf) return { problem: t(`فایل «${file.name}» بزرگ‌تر از ${fmtSize(caps.max_pdf)} است.`, `The file "${file.name}" is larger than ${fmtSize(caps.max_pdf)}.`) };
    return { kind: "pdf" };
  }
  if (ext === "zip" || mime === "application/zip" || mime === "application/x-zip-compressed") {
    if (file.size > caps.max_archive) return { problem: t(`فایل «${file.name}» بزرگ‌تر از ${fmtSize(caps.max_archive)} است.`, `The file "${file.name}" is larger than ${fmtSize(caps.max_archive)}.`) };
    return { kind: "archive" };
  }
  if (caps.text.includes(ext) || caps.text_names.includes(base) || mime.startsWith("text/")) {
    if (file.size > caps.max_text) return { problem: t(`فایل «${file.name}» بزرگ‌تر از ${fmtSize(caps.max_text)} است.`, `The file "${file.name}" is larger than ${fmtSize(caps.max_text)}.`) };
    return { kind: "text" };
  }
  if (ext === "rar" || ext === "7z" || ext === "tar" || ext === "gz") return { problem: t(`فایل «${file.name}» را به ZIP تبدیل کنید؛ فقط ZIP باز می‌شود.`, `Convert "${file.name}" to ZIP; only ZIP archives can be opened.`) };
  return { problem: t(`قالب فایل «${file.name || t("الحاقی", "clipboard")}» پشتیبانی نمی‌شود. تصویر، فایل متنی یا کد، و ZIP پذیرفته می‌شود.`, `The format of "${file.name || "clipboard"}" is not supported. Images, text or code files, and ZIP are accepted.`) };
}

const encoder = new TextEncoder();
const decoder = new TextDecoder();

/** The API counts bytes, not UTF-16 characters, so limits must be byte based. */
export function textBytes(text: string): number {
  return encoder.encode(text).length;
}

export function textFile(name: string, text: string): File {
  return new File([text], name, { type: "text/plain;charset=utf-8" });
}

/** Split on UTF-8 boundaries so every part is still valid text. */
export function splitText(text: string, maxBytes: number): string[] {
  const limit = Math.max(4, maxBytes);
  const bytes = encoder.encode(text);
  const parts: string[] = [];
  let at = 0;
  while (at < bytes.length) {
    let end = Math.min(at + limit, bytes.length);
    if (end < bytes.length) {
      while (end > at && (bytes[end] & 0xc0) === 0x80) end--;
      if (end === at) end = Math.min(at + limit, bytes.length);
    }
    parts.push(decoder.decode(bytes.subarray(at, end)));
    at = end;
  }
  return parts.length > 0 ? parts : [""];
}

export function readBase64(file: File): Promise<string> {
  return new Promise((ok, fail) => {
    const r = new FileReader();
    r.onload = () => {
      const url = String(r.result);
      ok(url.slice(url.indexOf(",") + 1));
    };
    r.onerror = () => fail(r.error);
    r.readAsDataURL(file);
  });
}

export async function toOutgoing(list: Pending[]): Promise<OutgoingFile[]> {
  return Promise.all(list.map(async (p) => ({ name: p.file.name, data: await readBase64(p.file) })));
}

function KindIcon({ kind }: { kind: FileMeta["kind"] }) {
  return kind === "archive" ? <IconFolder width={16} height={16} /> : <IconFile width={16} height={16} />;
}

export function PendingChips({ items, onRemove }: { items: Pending[]; onRemove: (key: string) => void }) {
  if (items.length === 0) return null;
  return (
    <ul className="attach-chips">
      {items.map((p) => (
        <li key={p.key} className={`attach-chip ${p.kind}`}>
          {p.preview ? <img src={p.preview} alt="" /> : <KindIcon kind={p.kind} />}
          <span className="attach-name" dir="auto" title={p.file.name}>
            {p.file.name}
          </span>
          <span className="attach-size">{fmtSize(p.file.size)}</span>
          <button type="button" className="attach-remove" onClick={() => onRemove(p.key)} aria-label={t(`حذف ${p.file.name}`, `Remove ${p.file.name}`)}>
            <IconClose width={14} height={14} />
          </button>
        </li>
      ))}
    </ul>
  );
}

function useFileSrc(f: FileMeta, load: (id: number) => Promise<string>): string {
  const [src, setSrc] = useState(f.url ?? "");
  useEffect(() => {
    if (src || f.id <= 0) return;
    let live = true;
    load(f.id)
      .then((u) => live && setSrc(u))
      .catch(() => {});
    return () => {
      live = false;
    };
  }, [f.id, src, load]);
  return src;
}

function ImageThumb({ f, load, onOpen }: { f: FileMeta; load: (id: number) => Promise<string>; onOpen: (src: string) => void }) {
  const src = useFileSrc(f, load);
  return src ? (
    <button type="button" className="attach-thumb" title={f.name} aria-label={t(`باز کردن ${f.name}`, `Open ${f.name}`)} onClick={() => onOpen(src)}>
      <img src={src} alt={f.name} draggable={false} data-user-image />
    </button>
  ) : (
    <span className="attach-thumb loading" title={f.name} />
  );
}

function FileChip({ f, load }: { f: FileMeta; load: (id: number) => Promise<string> }) {
  const [busy, setBusy] = useState(false);
  const save = async () => {
    if (busy || f.id <= 0) return;
    setBusy(true);
    try {
      downloadUrl(await load(f.id), f.name);
    } catch {
      pushToast({ tone: "failed", title: t("دریافت فایل ناموفق بود.", "Could not download the file.") });
    } finally {
      setBusy(false);
    }
  };
  return (
    <li className={`attach-chip ${f.kind}${f.id > 0 ? " file-button" : ""}`} onClick={() => void save()}>
      <KindIcon kind={f.kind} />
      <span className="attach-name" dir="auto" title={f.name}>
        {f.name}
      </span>
      <span className="attach-size">{fmtSize(f.size)}</span>
      {f.id > 0 && (
        <button type="button" className="attach-dl" disabled={busy} aria-label={t(`دریافت ${f.name}`, `Download ${f.name}`)} title={t("دریافت فایل", "Download file")} onClick={(e) => {
          e.stopPropagation();
          void save();
        }}>
          {busy ? <span className="spinner tiny" /> : <IconDownload width={16} height={16} />}
        </button>
      )}
    </li>
  );
}

export function MessageFiles({ files, load }: { files: FileMeta[]; load: (id: number) => Promise<string> }) {
  const images = files.filter((f) => f.kind === "image");
  const others = files.filter((f) => f.kind !== "image");
  const [open, setOpen] = useState<number | null>(null);
  const [srcs, setSrcs] = useState<Record<number, string>>({});
  return (
    <div className="msg-files">
      {images.length > 0 && (
        <div className="msg-images">
          {images.map((f) => (
            <ImageThumb
              key={f.id}
              f={f}
              load={load}
              onOpen={(src) => {
                setSrcs((m) => ({ ...m, [f.id]: src }));
                setOpen(f.id);
              }}
            />
          ))}
        </div>
      )}
      {others.length > 0 && (
        <ul className="attach-chips">
          {others.map((f) => (
            <FileChip key={f.id} f={f} load={load} />
          ))}
        </ul>
      )}
      {open !== null && (
        <LightboxFor images={images} load={load} srcs={srcs} start={open} onClose={() => setOpen(null)} />
      )}
    </div>
  );
}

function LightboxFor({ images, load, srcs, start, onClose }: { images: FileMeta[]; load: (id: number) => Promise<string>; srcs: Record<number, string>; start: number; onClose: () => void }) {
  const [urls, setUrls] = useState<Record<number, string>>(srcs);
  const [index, setIndex] = useState(Math.max(0, images.findIndex((f) => f.id === start)));
  useEffect(() => {
    let live = true;
    images.forEach((f) => {
      if (urls[f.id] || f.url || f.id <= 0) return;
      load(f.id)
        .then((u) => live && setUrls((m) => ({ ...m, [f.id]: u })))
        .catch(() => {});
    });
    return () => {
      live = false;
    };
  }, [images, load, urls]);
  const items = images.map((f) => ({ src: urls[f.id] ?? f.url ?? "", name: f.name }));
  return <Lightbox items={items} index={index} onIndex={setIndex} onClose={onClose} />;
}

export function fileLoader(fetcher: (id: number) => Promise<{ mime: string; data: string }>): (id: number) => Promise<string> {
  const cache = new Map<number, Promise<string>>();
  return (id) => {
    let p = cache.get(id);
    if (!p) {
      p = fetcher(id).then((r) => {
        const bytes = Uint8Array.from(atob(r.data), (c) => c.charCodeAt(0));
        return URL.createObjectURL(new Blob([bytes], { type: r.mime }));
      });
      p.catch(() => cache.delete(id));
      cache.set(id, p);
    }
    return p;
  };
}
