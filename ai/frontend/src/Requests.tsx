import { useState } from "react";
import type { ApiRequest, RequestStatus } from "./api";
import { fmtDateTime, fmtDuration, fmtNum, fmtRelative } from "./format";
import { IconChevronDown } from "./icons";
import { CopyButton } from "./Markdown";

const STATUS: Record<RequestStatus, { label: string; badge: string; dot: string }> = {
  done: { label: "موفق", badge: "ok", dot: "ok" },
  error: { label: "خطا", badge: "danger", dot: "error" },
  aborted: { label: "نیمه‌کاره", badge: "", dot: "idle" },
  pending: { label: "در حال پاسخ", badge: "pending", dot: "pending" },
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
      <pre dir="auto">{text || <span className="muted">خالی</span>}</pre>
    </div>
  );
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
  return (
    <li className="event request">
      <div className="event-row">
        <span className={`status-dot ${st.dot}`} aria-hidden="true" />
        {onKey ? (
          <button type="button" className="link request-key" onClick={() => onKey(r.key.id, r.key.name)} title="نمایش درخواست‌های این کلید">
            <bdi>{r.key.name}</bdi>
          </button>
        ) : (
          <strong className="request-key">
            <bdi>{r.key.name}</bdi>
          </strong>
        )}
        {r.project &&
          (onProject ? (
            <button type="button" className="badge project-badge" onClick={() => onProject(r.project)} title="نمایش درخواست‌های این پروژه">
              پروژه: <bdi>{r.project}</bdi>
            </button>
          ) : (
            <span className="badge project-badge">
              پروژه: <bdi>{r.project}</bdi>
            </span>
          ))}
        <span className={`badge ${st.badge}`}>{st.label}</span>
        {r.status !== "pending" && <span className="event-meta">{fmtDuration(r.ms)}</span>}
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
          {preview(r.prompt) || <span className="muted">پرسش خالی</span>}
        </p>
        <button type="button" className="button ghost small" aria-expanded={open} aria-controls={open ? detailId : undefined} onClick={() => setOpen(!open)}>
          {open ? "بستن" : "جزئیات"}
          <IconChevronDown width={16} height={16} className={`chevron ${open ? "up" : ""}`} />
        </button>
      </div>
      {open && (
        <div id={detailId} className="request-detail">
          <dl className="request-facts">
            <div>
              <dt>مدل</dt>
              <dd dir="ltr">{r.model}</dd>
            </div>
            <div>
              <dt>پاسخ جریانی</dt>
              <dd>{r.stream ? "بله" : "خیر"}</dd>
            </div>
            <div>
              <dt>نویسه‌ها</dt>
              <dd>
                {fmtNum(r.prompt_chars)} پرسش، {fmtNum(r.completion_chars)} پاسخ
              </dd>
            </div>
            {r.end_user && (
              <div>
                <dt>کاربر نهایی</dt>
                <dd dir="ltr">{r.end_user}</dd>
              </div>
            )}
            <div>
              <dt>زمان</dt>
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
                <dt>کلاینت</dt>
                <dd dir="ltr">{r.agent}</dd>
              </div>
            )}
            <div>
              <dt>شناسه</dt>
              <dd dir="ltr">{r.id}</dd>
            </div>
          </dl>
          <TextBlock title="پرسش" text={r.prompt} />
          <TextBlock title="پاسخ" text={r.response} />
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
  if (requests.length === 0 && !loading) return <p className="empty-list">{empty || "هنوز درخواستی ثبت نشده است."}</p>;
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
          <span className="sr-only">در حال بارگذاری</span>
        </div>
      )}
      {hasMore && !loading && (
        <button type="button" className="button ghost more" onClick={onMore}>
          نمایش موارد بیشتر
        </button>
      )}
    </div>
  );
}
