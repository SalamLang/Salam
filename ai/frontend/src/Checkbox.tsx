import { useEffect, useRef } from "react";
import type { ReactNode } from "react";

type Props = {
  checked: boolean;
  onChange: (checked: boolean) => void;
  label?: ReactNode;
  hint?: ReactNode;
  disabled?: boolean;
  indeterminate?: boolean;
  className?: string;
  name?: string;
  id?: string;
  ariaLabel?: string;
};

export function Checkbox({ checked, onChange, label, hint, disabled, indeterminate, className, name, id, ariaLabel }: Props) {
  const ref = useRef<HTMLInputElement>(null);
  useEffect(() => {
    if (ref.current) ref.current.indeterminate = !!indeterminate && !checked;
  }, [indeterminate, checked]);
  return (
    <label className={"cb" + (className ? " " + className : "")} data-disabled={disabled ? "true" : undefined}>
      <input
        ref={ref}
        className="cb-input"
        type="checkbox"
        id={id}
        name={name}
        checked={checked}
        disabled={disabled}
        aria-label={ariaLabel}
        onChange={(e) => onChange(e.target.checked)}
      />
      <span className="cb-box" aria-hidden="true">
        <svg viewBox="0 0 16 16" fill="none" stroke="currentColor" strokeWidth="2.2" strokeLinecap="round" strokeLinejoin="round">
          <path className="cb-tick" d="M3.5 8.6l3 3 6-7.2" />
          <path className="cb-dash" d="M4 8h8" />
        </svg>
      </span>
      {(label || hint) && (
        <span className="cb-text">
          {label && <span className="cb-label">{label}</span>}
          {hint && <span className="cb-hint">{hint}</span>}
        </span>
      )}
    </label>
  );
}
