import { useEffect, useState } from "react";
import { api, type Capabilities, type FileMeta, type OutgoingFile } from "./api";
import { IconClose, IconFile, IconFolder } from "./icons";
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
  if (IMAGE_EXTS.includes(ext)) {
    if (!caps.image) return { problem: t("مدل فعلی دستیار تصویر نمی‌پذیرد.", "The current assistant model does not accept images.") };
    if (file.size > caps.max_image) return { problem: t(`تصویر «${file.name}» بزرگ‌تر از ${fmtSize(caps.max_image)} است.`, `The image "${file.name}" is larger than ${fmtSize(caps.max_image)}.`) };
    return { kind: "image" };
  }
  if (ext === "pdf") {
    if (!caps.pdf) return { problem: t("مدل فعلی دستیار فایل PDF نمی‌پذیرد. متن آن را در یک فایل متنی بفرستید.", "The current assistant model does not accept PDF files. Send its text in a text file instead.") };
    if (file.size > caps.max_pdf) return { problem: t(`فایل «${file.name}» بزرگ‌تر از ${fmtSize(caps.max_pdf)} است.`, `The file "${file.name}" is larger than ${fmtSize(caps.max_pdf)}.`) };
    return { kind: "pdf" };
  }
  if (ext === "zip") {
    if (file.size > caps.max_archive) return { problem: t(`فایل «${file.name}» بزرگ‌تر از ${fmtSize(caps.max_archive)} است.`, `The file "${file.name}" is larger than ${fmtSize(caps.max_archive)}.`) };
    return { kind: "archive" };
  }
  if (caps.text.includes(ext) || caps.text_names.includes(base)) {
    if (file.size > caps.max_text) return { problem: t(`فایل «${file.name}» بزرگ‌تر از ${fmtSize(caps.max_text)} است.`, `The file "${file.name}" is larger than ${fmtSize(caps.max_text)}.`) };
    return { kind: "text" };
  }
  if (ext === "rar" || ext === "7z" || ext === "tar" || ext === "gz") return { problem: t(`فایل «${file.name}» را به ZIP تبدیل کنید؛ فقط ZIP باز می‌شود.`, `Convert "${file.name}" to ZIP; only ZIP archives can be opened.`) };
  return { problem: t(`قالب فایل «${file.name}» پشتیبانی نمی‌شود. تصویر، فایل متنی یا کد، و ZIP پذیرفته می‌شود.`, `The format of "${file.name}" is not supported. Images, text or code files, and ZIP are accepted.`) };
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

function ImageThumb({ f, load }: { f: FileMeta; load: (id: number) => Promise<string> }) {
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
  return src ? (
    <a href={src} target="_blank" rel="noopener" className="attach-thumb" title={f.name}>
      <img src={src} alt={f.name} />
    </a>
  ) : (
    <span className="attach-thumb loading" title={f.name} />
  );
}

export function MessageFiles({ files, load }: { files: FileMeta[]; load: (id: number) => Promise<string> }) {
  const images = files.filter((f) => f.kind === "image");
  const others = files.filter((f) => f.kind !== "image");
  return (
    <div className="msg-files">
      {images.length > 0 && (
        <div className="msg-images">
          {images.map((f) => (
            <ImageThumb key={f.id} f={f} load={load} />
          ))}
        </div>
      )}
      {others.length > 0 && (
        <ul className="attach-chips">
          {others.map((f) => (
            <li key={f.id} className={`attach-chip ${f.kind}`}>
              <KindIcon kind={f.kind} />
              <span className="attach-name" dir="auto" title={f.name}>
                {f.name}
              </span>
              <span className="attach-size">{fmtSize(f.size)}</span>
            </li>
          ))}
        </ul>
      )}
    </div>
  );
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
