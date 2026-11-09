import { useEffect, useState } from "react";
import { api, ApiError } from "./api";
import { fmtNum } from "./format";
import { t } from "./i18n";

export const ACCOUNT_MASTER_MAX = 2000;
export const PROJECT_MASTER_MAX = 4000;

function size(s: string): number {
  return Array.from(s).length;
}

function cut(s: string, max: number): string {
  const chars = Array.from(s);
  return chars.length > max ? chars.slice(0, max).join("") : s;
}

type FieldProps = {
  value: string;
  max: number;
  onChange: (v: string) => void;
  placeholder: string;
  rows?: number;
  id?: string;
};

export function MasterField({ value, max, onChange, placeholder, rows = 5, id }: FieldProps) {
  const n = size(value);
  return (
    <>
      <textarea
        id={id}
        className="project-textarea master-textarea"
        dir="auto"
        rows={rows}
        value={value}
        placeholder={placeholder}
        onChange={(e) => onChange(cut(e.target.value, max))}
      />
      <span className={`master-count ${n >= max ? "full" : ""}`} aria-live="polite">
        {fmtNum(n)} / {fmtNum(max)}
      </span>
    </>
  );
}

export function AccountMaster({ notify }: { notify: (msg: string, ok?: boolean) => void }) {
  const [saved, setSaved] = useState("");
  const [value, setValue] = useState("");
  const [ready, setReady] = useState(false);
  const [busy, setBusy] = useState(false);

  useEffect(() => {
    let live = true;
    api
      .masterPrompt()
      .then((r) => {
        if (!live) return;
        setSaved(r.master_prompt);
        setValue(r.master_prompt);
        setReady(true);
      })
      .catch(() => {
        if (live) setReady(true);
      });
    return () => {
      live = false;
    };
  }, []);

  const save = async () => {
    setBusy(true);
    try {
      const r = await api.setMasterPrompt(value.trim());
      setSaved(r.master_prompt);
      setValue(r.master_prompt);
      notify(t("پرامپت اصلی حساب ذخیره شد.", "Account master prompt saved."), true);
    } catch (e) {
      notify(e instanceof ApiError ? e.message : t("ذخیره ناموفق بود.", "Could not save."));
    } finally {
      setBusy(false);
    }
  };

  return (
    <section className="card panel master-card">
      <div className="panel-title">
        <h3>{t("پرامپت اصلی حساب", "Account master prompt")}</h3>
      </div>
      <p className="muted">
        {t(
          "در همه‌ی گفت‌وگوها و پروژه‌های شما به دستیار داده می‌شود. پرامپت اصلی هر پروژه از این بالاتر است. هر دو پایین‌تر از قوانین خود سرویس هستند و نمی‌توانند آن‌ها را بردارند.",
          "Given to the assistant in all your chats and projects. A project's own master prompt ranks above this one. Both rank below the service's own rules and cannot lift them.",
        )}
      </p>
      <label className="field">
        <span>{t("چگونه با شما کار کند", "How it should work with you")}</span>
        <MasterField
          value={value}
          max={ACCOUNT_MASTER_MAX}
          onChange={setValue}
          rows={5}
          placeholder={t("مثلاً: کوتاه و دقیق جواب بده و اول برنامه را بگو.", "For example: answer briefly and state the plan first.")}
        />
      </label>
      <div className="memory-actions">
        <button type="button" className="button primary small" onClick={save} disabled={!ready || busy || value.trim() === saved}>
          {t("ذخیره", "Save")}
        </button>
      </div>
    </section>
  );
}
