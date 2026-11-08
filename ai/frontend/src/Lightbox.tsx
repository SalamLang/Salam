import { useCallback, useEffect, useRef, useState } from "react";
import { IconChevronLeft, IconChevronRight, IconClose, IconDownload, IconFit, IconZoomIn, IconZoomOut } from "./icons";
import { toPersianDigits } from "./theme";
import { t } from "./i18n";

export type LightboxItem = { src: string; name: string };

type View = { s: number; x: number; y: number };

const MAX_SCALE = 10;
const STEP = 1.5;
const TAP_MS = 320;
const TAP_PX = 28;

export function downloadUrl(url: string, name: string) {
  const a = document.createElement("a");
  a.href = url;
  a.download = name;
  document.body.appendChild(a);
  a.click();
  a.remove();
}

function clamp(v: number, lo: number, hi: number): number {
  return Math.min(hi, Math.max(lo, v));
}

export function Lightbox({ items, index, onIndex, onClose }: { items: LightboxItem[]; index: number; onIndex: (i: number) => void; onClose: () => void }) {
  const item = items[index];
  const stage = useRef<HTMLDivElement>(null);
  const image = useRef<HTMLImageElement>(null);
  const closeBtn = useRef<HTMLButtonElement>(null);
  const view = useRef<View>({ s: 1, x: 0, y: 0 });
  const [shown, setShown] = useState<View>({ s: 1, x: 0, y: 0 });
  const [smooth, setSmooth] = useState(false);
  const [loaded, setLoaded] = useState(false);
  const pointers = useRef(new Map<number, { x: number; y: number }>());
  const gesture = useRef<{ dist: number; mx: number; my: number; base: View } | null>(null);
  const drag = useRef<{ x: number; y: number; moved: boolean; at: number } | null>(null);
  const lastTap = useRef<{ x: number; y: number; at: number } | null>(null);
  const swipe = useRef(0);

  const bounds = useCallback((s: number) => {
    const st = stage.current;
    const im = image.current;
    if (!st || !im) return { mx: 0, my: 0 };
    const W = st.clientWidth;
    const H = st.clientHeight;
    return { mx: Math.max(0, (im.offsetWidth * s - W) / 2), my: Math.max(0, (im.offsetHeight * s - H) / 2) };
  }, []);

  const apply = useCallback(
    (next: View, animate = false) => {
      const s = clamp(next.s, 1, MAX_SCALE);
      const b = bounds(s);
      const v = { s, x: clamp(next.x, -b.mx, b.mx), y: clamp(next.y, -b.my, b.my) };
      view.current = v;
      setSmooth(animate);
      setShown(v);
    },
    [bounds],
  );

  const zoomAt = useCallback(
    (factor: number, cx: number, cy: number, animate = false) => {
      const st = stage.current;
      if (!st) return;
      const r = st.getBoundingClientRect();
      const px = cx - (r.left + r.width / 2);
      const py = cy - (r.top + r.height / 2);
      const v = view.current;
      const s = clamp(v.s * factor, 1, MAX_SCALE);
      const k = s / v.s;
      apply({ s, x: px - (px - v.x) * k, y: py - (py - v.y) * k }, animate);
    },
    [apply],
  );

  const zoomCenter = useCallback(
    (factor: number) => {
      const st = stage.current;
      if (!st) return;
      const r = st.getBoundingClientRect();
      zoomAt(factor, r.left + r.width / 2, r.top + r.height / 2, true);
    },
    [zoomAt],
  );

  const reset = useCallback(() => apply({ s: 1, x: 0, y: 0 }, true), [apply]);

  const go = useCallback(
    (d: number) => {
      if (items.length < 2) return;
      onIndex((index + d + items.length) % items.length);
    },
    [index, items.length, onIndex],
  );

  useEffect(() => {
    view.current = { s: 1, x: 0, y: 0 };
    setShown({ s: 1, x: 0, y: 0 });
    setLoaded(false);
    pointers.current.clear();
    gesture.current = null;
    drag.current = null;
  }, [index]);

  useEffect(() => {
    const prev = document.body.style.overflow;
    document.body.style.overflow = "hidden";
    const opener = document.activeElement as HTMLElement | null;
    closeBtn.current?.focus();
    return () => {
      document.body.style.overflow = prev;
      opener?.focus?.();
    };
  }, []);

  useEffect(() => {
    const onKey = (e: KeyboardEvent) => {
      if (e.key === "Escape") {
        e.preventDefault();
        onClose();
      } else if (e.key === "+" || e.key === "=") zoomCenter(STEP);
      else if (e.key === "-" || e.key === "_") zoomCenter(1 / STEP);
      else if (e.key === "0") reset();
      else if (e.key === "ArrowLeft") {
        if (view.current.s > 1) apply({ ...view.current, x: view.current.x + 80 });
        else go(-1);
      } else if (e.key === "ArrowRight") {
        if (view.current.s > 1) apply({ ...view.current, x: view.current.x - 80 });
        else go(1);
      } else if (e.key === "ArrowUp" && view.current.s > 1) apply({ ...view.current, y: view.current.y + 80 });
      else if (e.key === "ArrowDown" && view.current.s > 1) apply({ ...view.current, y: view.current.y - 80 });
    };
    window.addEventListener("keydown", onKey);
    return () => window.removeEventListener("keydown", onKey);
  }, [apply, go, onClose, reset, zoomCenter]);

  useEffect(() => {
    const el = stage.current;
    if (!el) return;
    const onWheel = (e: WheelEvent) => {
      e.preventDefault();
      const unit = e.deltaMode === 1 ? 33 : 1;
      const k = Math.exp((-e.deltaY * unit * (e.ctrlKey ? 0.01 : 0.0018)));
      zoomAt(k, e.clientX, e.clientY);
    };
    el.addEventListener("wheel", onWheel, { passive: false });
    return () => el.removeEventListener("wheel", onWheel);
  }, [zoomAt]);

  useEffect(() => {
    const onResize = () => apply(view.current);
    window.addEventListener("resize", onResize);
    return () => window.removeEventListener("resize", onResize);
  }, [apply]);

  const startGesture = () => {
    const pts = [...pointers.current.values()];
    if (pts.length >= 2) {
      const [a, b] = pts;
      gesture.current = { dist: Math.hypot(a.x - b.x, a.y - b.y) || 1, mx: (a.x + b.x) / 2, my: (a.y + b.y) / 2, base: { ...view.current } };
      drag.current = null;
    } else gesture.current = null;
  };

  const onDown = (e: React.PointerEvent) => {
    if (e.pointerType === "mouse" && e.button !== 0) return;
    if ((e.target as HTMLElement).closest("button")) return;
    (e.currentTarget as HTMLElement).setPointerCapture(e.pointerId);
    pointers.current.set(e.pointerId, { x: e.clientX, y: e.clientY });
    setSmooth(false);
    if (pointers.current.size >= 2) startGesture();
    else {
      drag.current = { x: e.clientX, y: e.clientY, moved: false, at: Date.now() };
      swipe.current = 0;
    }
  };

  const onMove = (e: React.PointerEvent) => {
    const p = pointers.current.get(e.pointerId);
    if (!p) return;
    const dx = e.clientX - p.x;
    const dy = e.clientY - p.y;
    p.x = e.clientX;
    p.y = e.clientY;
    const g = gesture.current;
    if (g && pointers.current.size >= 2) {
      const [a, b] = [...pointers.current.values()];
      const dist = Math.hypot(a.x - b.x, a.y - b.y) || 1;
      const mx = (a.x + b.x) / 2;
      const my = (a.y + b.y) / 2;
      const st = stage.current;
      if (!st) return;
      const r = st.getBoundingClientRect();
      const cx0 = g.mx - (r.left + r.width / 2);
      const cy0 = g.my - (r.top + r.height / 2);
      const s = clamp(g.base.s * (dist / g.dist), 1, MAX_SCALE);
      const k = s / g.base.s;
      const cx1 = mx - (r.left + r.width / 2);
      const cy1 = my - (r.top + r.height / 2);
      apply({ s, x: cx1 - (cx0 - g.base.x) * k, y: cy1 - (cy0 - g.base.y) * k });
      return;
    }
    const d = drag.current;
    if (!d) return;
    if (Math.hypot(e.clientX - d.x, e.clientY - d.y) > 6) d.moved = true;
    if (view.current.s > 1) apply({ s: view.current.s, x: view.current.x + dx, y: view.current.y + dy });
    else swipe.current += dx;
  };

  const onUp = (e: React.PointerEvent) => {
    const had = pointers.current.delete(e.pointerId);
    if (!had) return;
    if (pointers.current.size >= 1) {
      startGesture();
      return;
    }
    gesture.current = null;
    const d = drag.current;
    drag.current = null;
    if (!d) return;
    const now = Date.now();
    if (!d.moved && now - d.at < 400) {
      const box = image.current?.getBoundingClientRect();
      const onImage = Boolean(box && e.clientX >= box.left && e.clientX <= box.right && e.clientY >= box.top && e.clientY <= box.bottom);
      const lt = lastTap.current;
      if (lt && now - lt.at < TAP_MS && Math.hypot(e.clientX - lt.x, e.clientY - lt.y) < TAP_PX) {
        lastTap.current = null;
        if (view.current.s > 1.05) reset();
        else zoomAt(2.5, e.clientX, e.clientY, true);
        return;
      }
      lastTap.current = { x: e.clientX, y: e.clientY, at: now };
      if (!onImage && view.current.s <= 1.05) {
        window.setTimeout(() => {
          if (lastTap.current && lastTap.current.at === now) onClose();
        }, TAP_MS + 20);
      }
      return;
    }
    if (view.current.s <= 1.05 && Math.abs(swipe.current) > 60 && items.length > 1) go(swipe.current < 0 ? 1 : -1);
  };

  if (!item) return null;
  const pct = Math.round(shown.s * 100);
  const label = items.length > 1 ? `${toPersianDigits(index + 1)} / ${toPersianDigits(items.length)}` : "";
  return (
    <div className="lightbox" role="dialog" aria-modal="true" aria-label={t("نمایش تصویر", "Image viewer")} dir="ltr">
      <div className="lightbox-bar">
        <span className="lightbox-title" dir="auto" title={item.name}>
          {label && <b>{label}</b>} {item.name}
        </span>
        <div className="lightbox-tools">
          <button type="button" onClick={() => zoomCenter(1 / STEP)} disabled={shown.s <= 1} aria-label={t("کوچک‌نمایی", "Zoom out")} title={t("کوچک‌نمایی (-)", "Zoom out (-)")}>
            <IconZoomOut width={20} height={20} />
          </button>
          <button type="button" className="lightbox-pct" onClick={reset} aria-label={t("اندازهٔ اصلی", "Fit to screen")} title={t("اندازهٔ صفحه (0)", "Fit to screen (0)")}>
            {toPersianDigits(pct)}%
          </button>
          <button type="button" onClick={() => zoomCenter(STEP)} disabled={shown.s >= MAX_SCALE} aria-label={t("بزرگ‌نمایی", "Zoom in")} title={t("بزرگ‌نمایی (+)", "Zoom in (+)")}>
            <IconZoomIn width={20} height={20} />
          </button>
          <button type="button" onClick={reset} aria-label={t("اندازهٔ صفحه", "Fit to screen")} title={t("اندازهٔ صفحه (0)", "Fit to screen (0)")}>
            <IconFit width={20} height={20} />
          </button>
          <button type="button" disabled={!item.src} onClick={() => downloadUrl(item.src, item.name)} aria-label={t("دریافت تصویر", "Download image")} title={t("دریافت تصویر", "Download image")}>
            <IconDownload width={20} height={20} />
          </button>
          <button type="button" ref={closeBtn} onClick={onClose} aria-label={t("بستن", "Close")} title={t("بستن (Esc)", "Close (Esc)")}>
            <IconClose width={20} height={20} />
          </button>
        </div>
      </div>
      <div
        ref={stage}
        className={`lightbox-stage${shown.s > 1 ? " zoomed" : ""}`}
        onPointerDown={onDown}
        onPointerMove={onMove}
        onPointerUp={onUp}
        onPointerCancel={onUp}
        onContextMenu={(e) => e.preventDefault()}
      >
        {(!loaded || !item.src) && <span className="spinner lightbox-spinner" />}
        {item.src && <img
          ref={image}
          src={item.src}
          alt={item.name}
          draggable={false}
          onLoad={() => setLoaded(true)}
          className={smooth ? "smooth" : undefined}
          style={{ transform: `translate(calc(-50% + ${shown.x}px), calc(-50% + ${shown.y}px)) scale(${shown.s})` }}
        />}
      </div>
      {items.length > 1 && (
        <>
          <button type="button" className="lightbox-nav prev" onClick={() => go(-1)} aria-label={t("تصویر قبلی", "Previous image")}>
            <IconChevronLeft width={26} height={26} />
          </button>
          <button type="button" className="lightbox-nav next" onClick={() => go(1)} aria-label={t("تصویر بعدی", "Next image")}>
            <IconChevronRight width={26} height={26} />
          </button>
        </>
      )}
    </div>
  );
}
