import { useState } from "react";
import type { ApiRequest, RequestStatus } from "./api";
import { fmtCompact, fmtCost, fmtDateTime, fmtDuration, fmtNum, fmtRelative, tokenSplit } from "./format";
import { IconChevronDown } from "./icons";
import { CopyButton } from "./Markdown";
import { t } from "./i18n";

const STATUS: Record<RequestStatus, { label: string; badge: string; dot: string }> = {
  done: { label: t("موفق", "Done"), badge: "ok", dot: "ok" },
  error: { label: t("خطا", "Error"), badge: "danger", dot: "error" },
  aborted: { label: t("نیمه‌کاره", "Aborted"), badge: "", dot: "idle" },
  pending: { label: t("در حال پاسخ", "Answering"), badge: "pending", dot: "pending" },
  lost: { label: t("قطع سرویس", "Service lost"), badge: "danger", dot: "error" },
};

export function preview(text: string, size = 140): string {
  const chars = Array.from(text.replace(/\s+/g, " ").trim());
  return chars.length > size ? `${chars.slice(0, size).join("").trimEnd()}…` : chars.join("");
}

function TextBlock({ title, text }: { title: string; text: string }) {
  return (
    <div className="code-block request-text">
      <div className="code-head">
        <span className="code-lang">{title}</span>
        {text && <CopyButton text={text} />}
      </div>
      <pre dir="auto">{text || <span className="muted">{t("خالی", "Empty")}</span>}</pre>
    </div>
  );
}

function usageSummary(r: ApiRequest): { text: string; title: string } | null {
  const u = r.usage;
  if (!u) return null;
  const split = tokenSplit(u);
  if (split.total === 0 && !u.tools) return null;
  const parts = [t(`${fmtCompact(split.input)} ورودی`, `${fmtCompact(split.input)} in`), t(`${fmtCompact(split.output)} خروجی`, `${fmtCompact(split.output)} out`)];
  const exact = [t(`${fmtNum(split.input)} توکن ورودی`, `${fmtNum(split.input)} input tokens`), t(`${fmtNum(split.output)} توکن خروجی`, `${fmtNum(split.output)} output tokens`)];
  if (split.cache > 0) {
    parts.push(t(`${fmtCompact(split.cache)} کش`, `${fmtCompact(split.cache)} cached`));
    exact.push(t(`${fmtNum(split.cache)} توکن کش`, `${fmtNum(split.cache)} cached tokens`));
  }
  if (u.tools > 0) {
    parts.push(t(`${fmtNum(u.tools)} ابزار`, `${fmtNum(u.tools)} tools`));
    exact.push(t(`${fmtNum(u.tools)} فراخوانی ابزار`, `${fmtNum(u.tools)} tool calls`));
  }
  const sep = t("، ", ", ");
  return { text: t(`توکن: ${parts.join(sep)}`, `Tokens: ${parts.join(sep)}`), title: exact.join(sep) };
}

type ItemProps = {
  r: ApiRequest;
  showUser?: boolean;
  onUser?: (id: number, name: string) => void;
  onKey?: (id: number, name: string) => void;
  onProject?: (name: string) => void;
};

function RequestItem({ r, showUser, onUser, onKey, onProject }: ItemProps) {
  const [open, setOpen] = useState(false);
  const st = STATUS[r.status] || STATUS.done;
  const detailId = `request-${r.id}`;
  const usage = usageSummary(r);
  const tk = r.usage ? tokenSplit(r.usage) : null;
  return (
    <li className="event request">
      <div className="event-row">
        <span className={`status-dot ${st.dot}`} aria-hidden="true" />
        {onKey ? (
          <button type="button" className="link request-key" onClick={() => onKey(r.key.id, r.key.name)} title={t("نمایش درخواست‌های این کلید", "Show this key's requests")}>
            <bdi>{r.key.name}</bdi>
          </button>
        ) : (
          <strong className="request-key">
            <bdi>{r.key.name}</bdi>
          </strong>
        )}
        {r.project &&
          (onProject ? (
            <button type="button" className="badge project-badge" onClick={() => onProject(r.project)} title={t("نمایش درخواست‌های این پروژه", "Show this project's requests")}>
              {t("پروژه:", "Project:")} <bdi>{r.project}</bdi>
            </button>
          ) : (
            <span className="badge project-badge">
              {t("پروژه:", "Project:")} <bdi>{r.project}</bdi>
            </span>
          ))}
        <span className={`badge ${st.badge}`}>{st.label}</span>
        {r.status !== "pending" && <span className="event-meta">{fmtDuration(r.ms)}</span>}
        {usage && (
          <span className="event-meta" title={usage.title}>
            {usage.text}
          </span>
        )}
        <time className="event-meta event-time" dateTime={new Date(r.created * 1000).toISOString()} title={fmtDateTime(r.created)}>
          {fmtRelative(r.created)}
        </time>
      </div>
      {showUser && r.user && (
        <div className="event-who">
          <span>
            {onUser && r.user.id !== 0 ? (
              <button type="button" className="link" onClick={() => r.user && onUser(r.user.id, r.user.name)}>
                <bdi>{r.user.name}</bdi>
              </button>
            ) : (
              <bdi>{r.user.name}</bdi>
            )}{" "}
            {r.user.email && (
              <span className="muted" dir="ltr">
                {r.user.email}
              </span>
            )}
          </span>
        </div>
      )}
      <div className="request-line">
        <p className="request-preview" dir="auto">
          {preview(r.prompt) || <span className="muted">{t("پرسش خالی", "Empty prompt")}</span>}
        </p>
        <button type="button" className="button ghost small" aria-expanded={open} aria-controls={open ? detailId : undefined} onClick={() => setOpen(!open)}>
          {open ? t("بستن", "Close") : t("جزئیات", "Details")}
          <IconChevronDown width={16} height={16} className={`chevron ${open ? "up" : ""}`} />
        </button>
      </div>
      {open && (
        <div id={detailId} className="request-detail">
          <dl className="request-facts">
            <div>
              <dt>{t("مدل", "Model")}</dt>
              <dd dir="ltr">{r.model}</dd>
            </div>
            <div>
              <dt>{t("پاسخ جریانی", "Streaming")}</dt>
              <dd>{r.stream ? t("بله", "Yes") : t("خیر", "No")}</dd>
            </div>
            <div>
              <dt>{t("نویسه‌ها", "Characters")}</dt>
              <dd>
                {t(`${fmtNum(r.prompt_chars)} پرسش، ${fmtNum(r.completion_chars)} پاسخ`, `${fmtNum(r.prompt_chars)} prompt, ${fmtNum(r.completion_chars)} response`)}
              </dd>
            </div>
            {r.usage && tk && (
              <>
                <div>
                  <dt>{t("توکن ورودی", "Input tokens")}</dt>
                  <dd>
                    {fmtNum(tk.input)}
                    {tk.cache > 0 && t(` (${fmtNum(r.usage.input)} تازه، ${fmtNum(r.usage.cache_read)} خوانده از کش، ${fmtNum(r.usage.cache_write)} نوشته در کش)`, ` (${fmtNum(r.usage.input)} fresh, ${fmtNum(r.usage.cache_read)} read from cache, ${fmtNum(r.usage.cache_write)} written to cache)`)}
                  </dd>
                </div>
                <div>
                  <dt>{t("توکن خروجی", "Output tokens")}</dt>
                  <dd>
                    {fmtNum(tk.output)}
                    {r.usage.reasoning > 0 && t(` (${fmtNum(r.usage.reasoning)} توکن استدلال)`, ` (${fmtNum(r.usage.reasoning)} reasoning tokens)`)}
                  </dd>
                </div>
                <div>
                  <dt>{t("جمع توکن‌ها", "Total tokens")}</dt>
                  <dd>{fmtNum(tk.total)}</dd>
                </div>
                <div>
                  <dt>{t("فراخوانی ابزار", "Tool calls")}</dt>
                  <dd>{fmtNum(r.usage.tools || 0)}</dd>
                </div>
                {r.usage.cost > 0 && (
                  <div>
                    <dt>{t("هزینه", "Cost")}</dt>
                    <dd>{fmtCost(r.usage.cost)}</dd>
                  </div>
                )}
              </>
            )}
            {r.end_user && (
              <div>
                <dt>{t("کاربر نهایی", "End user")}</dt>
                <dd dir="ltr">{r.end_user}</dd>
              </div>
            )}
            <div>
              <dt>{t("زمان", "Time")}</dt>
              <dd>{fmtDateTime(r.created)}</dd>
            </div>
            {r.ip && (
              <div>
                <dt>IP</dt>
                <dd dir="ltr">{r.ip}</dd>
              </div>
            )}
            {r.agent && (
              <div>
                <dt>{t("کلاینت", "Client")}</dt>
                <dd dir="ltr">{r.agent}</dd>
              </div>
            )}
            <div>
              <dt>{t("شناسه", "ID")}</dt>
              <dd dir="ltr">{r.id}</dd>
            </div>
          </dl>
          <TextBlock title={t("پرسش", "Prompt")} text={r.prompt} />
          <TextBlock title={t("پاسخ", "Response")} text={r.response} />
        </div>
      )}
    </li>
  );
}

type Props = {
  requests: ApiRequest[];
  loading: boolean;
  hasMore: boolean;
  onMore: () => void;
  showUser?: boolean;
  onUser?: (id: number, name: string) => void;
  onKey?: (id: number, name: string) => void;
  onProject?: (name: string) => void;
  empty?: string;
};

export function RequestList({ requests, loading, hasMore, onMore, empty, ...rest }: Props) {
  if (requests.length === 0 && !loading) return <p className="empty-list">{empty || t("هنوز درخواستی ثبت نشده است.", "No requests yet.")}</p>;
  return (
    <div className="events">
      <ul className="event-list">
        {requests.map((r) => (
          <RequestItem key={r.id} r={r} {...rest} />
        ))}
      </ul>
      {loading && (
        <div className="list-loading" role="status">
          <span className="spinner" />
          <span className="sr-only">{t("در حال بارگذاری", "Loading")}</span>
        </div>
      )}
      {hasMore && !loading && (
        <button type="button" className="button ghost more" onClick={onMore}>
          {t("نمایش موارد بیشتر", "Show more")}
        </button>
      )}
    </div>
  );
}
