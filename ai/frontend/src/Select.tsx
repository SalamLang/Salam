import {
  type ReactNode,
  useCallback,
  useEffect,
  useId,
  useLayoutEffect,
  useRef,
  useState,
} from "react";
import { createPortal } from "react-dom";
import { IconCheck, IconChevronDown } from "./icons";

export type SelectOption<T extends string> = {
  value: T;
  label: string;
  hint?: string;
  icon?: ReactNode;
  tone?: "default" | "danger";
  dividerBefore?: boolean;
};

type Props<T extends string> = {
  value: T;
  options: SelectOption<T>[];
  onChange: (value: T) => void;
  ariaLabel: string;
  className?: string;
  disabled?: boolean;
  title?: string;
};

type Place = {
  left: number;
  width: number;
  maxHeight: number;
  top?: number;
  bottom?: number;
};

const GAP = 6;
const MARGIN = 10;
const MAX_HEIGHT = 340;

export function Select<T extends string>({
  value,
  options,
  onChange,
  ariaLabel,
  className,
  disabled,
  title,
}: Props<T>) {
  const id = useId();
  const listId = `${id}-list`;
  const trigger = useRef<HTMLButtonElement>(null);
  const menu = useRef<HTMLDivElement>(null);
  const [open, setOpen] = useState(false);
  const [place, setPlace] = useState<Place | null>(null);
  const [active, setActive] = useState(0);
  const selected = Math.max(
    0,
    options.findIndex((o) => o.value === value),
  );

  const close = useCallback((focus = true) => {
    setOpen(false);
    if (focus) trigger.current?.focus();
  }, []);

  const pick = useCallback(
    (option: SelectOption<T>) => {
      close();
      if (option.value !== value) onChange(option.value);
    },
    [close, onChange, value],
  );

  const show = useCallback(() => {
    setActive(selected);
    setOpen(true);
  }, [selected]);

  useEffect(() => {
    if (!open) return;
    const inside = (node: Node | null) =>
      !!node &&
      (trigger.current?.contains(node) || menu.current?.contains(node));
    const away = (e: PointerEvent) => {
      if (!inside(e.target as Node)) close(false);
    };
    const blur = () => close(false);
    document.addEventListener("pointerdown", away, true);
    window.addEventListener("blur", blur);
    return () => {
      document.removeEventListener("pointerdown", away, true);
      window.removeEventListener("blur", blur);
    };
  }, [open, close]);

  useEffect(() => {
    if (!open) return;
    const onKey = (e: KeyboardEvent) => {
      if (e.key === "Escape") {
        e.preventDefault();
        e.stopPropagation();
        close();
        return;
      }
      if (e.key === "Tab") {
        close(false);
        return;
      }
      const step = e.key === "ArrowDown" ? 1 : e.key === "ArrowUp" ? -1 : 0;
      if (step !== 0) {
        e.preventDefault();
        setActive((i) => (i + step + options.length) % options.length);
        return;
      }
      if (e.key === "Home" || e.key === "End") {
        e.preventDefault();
        setActive(e.key === "Home" ? 0 : options.length - 1);
        return;
      }
      if (e.key === "Enter" || e.key === " ") {
        e.preventDefault();
        const option = options[active];
        if (option) pick(option);
      }
    };
    document.addEventListener("keydown", onKey, true);
    return () => document.removeEventListener("keydown", onKey, true);
  }, [open, options, active, pick, close]);

  useLayoutEffect(() => {
    if (!open) {
      setPlace(null);
      return;
    }
    const measure = () => {
      const el = trigger.current;
      const list = menu.current;
      if (!el) return;
      const r = el.getBoundingClientRect();
      const vw = window.innerWidth;
      const vh = window.innerHeight;
      const width = Math.min(Math.max(r.width, 288), vw - MARGIN * 2);
      const rtl = getComputedStyle(el).direction === "rtl";
      const wanted = Math.min(list?.scrollHeight ?? 300, MAX_HEIGHT);
      const below = vh - r.bottom - MARGIN;
      const above = r.top - MARGIN;
      const up = below < wanted + GAP && above > below;
      const left = Math.max(
        MARGIN,
        Math.min(rtl ? r.right - width : r.left, vw - MARGIN - width),
      );
      setPlace({
        left,
        width,
        maxHeight: Math.max(
          150,
          Math.min(MAX_HEIGHT, (up ? above : below) - GAP),
        ),
        ...(up ? { bottom: vh - r.top + GAP } : { top: r.bottom + GAP }),
      });
    };
    measure();
    window.addEventListener("resize", measure);
    window.addEventListener("scroll", measure, true);
    return () => {
      window.removeEventListener("resize", measure);
      window.removeEventListener("scroll", measure, true);
    };
  }, [open]);

  useEffect(() => {
    if (!open) return;
    const node = menu.current?.querySelector<HTMLElement>(
      `#${CSS.escape(`${listId}-${active}`)}`,
    );
    node?.scrollIntoView({ block: "nearest" });
  }, [open, active, listId]);

  const current = options[selected];
  const list = open
    ? createPortal(
        <div
          ref={menu}
          id={listId}
          className={`pick-menu${place ? " ready" : ""}`}
          role="listbox"
          aria-label={ariaLabel}
          style={
            place
              ? {
                  left: place.left,
                  width: place.width,
                  maxHeight: place.maxHeight,
                  top: place.top,
                  bottom: place.bottom,
                }
              : { visibility: "hidden" }
          }
        >
          {options.map((option, i) => (
            <div
              key={option.value}
              className={option.dividerBefore ? "pick-sep" : undefined}
            >
              <button
                type="button"
                id={`${listId}-${i}`}
                role="option"
                aria-selected={option.value === value}
                tabIndex={-1}
                className={`pick-option${i === active ? " active" : ""}${option.tone === "danger" ? " danger" : ""}`}
                onPointerMove={() => setActive(i)}
                onClick={() => pick(option)}
              >
                {option.icon && (
                  <span className="pick-option-icon">{option.icon}</span>
                )}
                <span className="pick-option-text">
                  <span className="pick-option-label">{option.label}</span>
                  {option.hint && (
                    <span className="pick-option-hint">{option.hint}</span>
                  )}
                </span>
                <span className="pick-check" aria-hidden="true">
                  {option.value === value && (
                    <IconCheck width={16} height={16} />
                  )}
                </span>
              </button>
            </div>
          ))}
        </div>,
        document.body,
      )
    : null;

  return (
    <div className={`pick${className ? ` ${className}` : ""}`}>
      <button
        ref={trigger}
        type="button"
        className={`pick-trigger${open ? " open" : ""}${current?.tone === "danger" ? " danger" : ""}`}
        role="combobox"
        aria-label={ariaLabel}
        aria-haspopup="listbox"
        aria-expanded={open}
        aria-controls={open ? listId : undefined}
        aria-activedescendant={open ? `${listId}-${active}` : undefined}
        title={title}
        disabled={disabled}
        onClick={() => (open ? close() : show())}
        onKeyDown={(e) => {
          if (e.key === "ArrowDown" || e.key === "ArrowUp") {
            e.preventDefault();
            if (!open) show();
          }
        }}
      >
        {current?.icon && (
          <span className="pick-trigger-icon">{current.icon}</span>
        )}
        <span className="pick-value">{current?.label ?? ""}</span>
        <IconChevronDown className="pick-chevron" width={14} height={14} />
      </button>
      {list}
    </div>
  );
}
