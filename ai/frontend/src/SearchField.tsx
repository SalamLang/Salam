import { useRef } from "react";
import type { KeyboardEvent } from "react";
import { IconClose, IconSearch } from "./icons";
import { t } from "./i18n";
import { autoDir } from "./dir";

type Props = {
  value: string;
  onChange: (value: string) => void;
  onSubmit?: (value: string) => void;
  placeholder: string;
  ariaLabel?: string;
  className?: string;
  id?: string;
  name?: string;
  autoFocus?: boolean;
  disabled?: boolean;
};

export function SearchField({ value, onChange, onSubmit, placeholder, ariaLabel, className, id, name, autoFocus, disabled }: Props) {
  const ref = useRef<HTMLInputElement>(null);
  const clear = () => {
    onChange("");
    onSubmit?.("");
    ref.current?.focus();
  };
  const onKey = (e: KeyboardEvent<HTMLInputElement>) => {
    if (e.key === "Escape" && value) {
      e.preventDefault();
      clear();
    } else if (e.key === "Enter" && onSubmit) {
      e.preventDefault();
      onSubmit(value.trim());
    }
  };
  return (
    <div className={"sf" + (className ? " " + className : "")} data-disabled={disabled ? "true" : undefined} data-filled={value ? "true" : undefined}>
      <IconSearch className="sf-icon" width={16} height={16} aria-hidden="true" />
      <input
        ref={ref}
        className="sf-input"
        type="text"
        role="searchbox"
        id={id}
        name={name}
        dir={autoDir(value, placeholder)}
        value={value}
        disabled={disabled}
        autoFocus={autoFocus}
        autoComplete="off"
        autoCorrect="off"
        spellCheck={false}
        enterKeyHint="search"
        placeholder={placeholder}
        aria-label={ariaLabel || placeholder}
        onChange={(e) => onChange(e.target.value)}
        onKeyDown={onKey}
      />
      {value && !disabled && (
        <button type="button" className="sf-clear" onClick={clear} aria-label={t("پاک کردن جست‌وجو", "Clear search")} title={t("پاک کردن", "Clear")}>
          <IconClose width={14} height={14} aria-hidden="true" />
        </button>
      )}
    </div>
  );
}
