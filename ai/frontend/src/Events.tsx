import type { AppEvent } from "./api";
import { fmtDateTime, fmtDuration, fmtRelative, prettyArgs } from "./format";
import { t } from "./i18n";

type Props = {
  events: AppEvent[];
  loading: boolean;
  hasMore: boolean;
  onMore: () => void;
  showUser?: boolean;
  showClient?: boolean;
  onUser?: (id: number, name: string) => void;
  empty?: string;
};

export function EventList({ events, loading, hasMore, onMore, showUser, showClient, onUser, empty }: Props) {
  if (events.length === 0 && !loading) return <p className="empty-list">{empty || t("هنوز فعالیتی ثبت نشده است.", "No activity yet.")}</p>;
  return (
    <div className="events">
      <ul className="event-list">
        {events.map((ev) => {
          const args = prettyArgs(ev.args);
          return (
            <li key={ev.id} className={`event ${ev.ok ? "ok" : "error"}`}>
              <div className="event-row">
                <span className={`status-dot ${ev.ok ? "ok" : "error"}`} aria-hidden="true" />
                <code className="event-tool" dir="ltr">
                  {ev.tool || ev.method}
                </code>
                <span className={`badge ${ev.ok ? "ok" : "danger"}`}>{ev.ok ? t("موفق", "OK") : t("خطا", "Error")}</span>
                <span className="event-meta">{fmtDuration(ev.ms)}</span>
                <time className="event-meta event-time" dateTime={new Date(ev.created * 1000).toISOString()} title={fmtDateTime(ev.created)}>
                  {fmtRelative(ev.created)}
                </time>
              </div>
              {(showUser || showClient) && (
                <div className="event-who">
                  {showUser && ev.user && (
                    <span>
                      {onUser && ev.user.id !== 0 ? (
                        <button type="button" className="link" onClick={() => onUser(ev.user!.id, ev.user!.name)}>
                          <bdi>{ev.user.name}</bdi>
                        </button>
                      ) : (
                        <bdi>{ev.user.name}</bdi>
                      )}{" "}
                      {ev.user.email && (
                        <span className="muted" dir="ltr">
                          {ev.user.email}
                        </span>
                      )}
                    </span>
                  )}
                  {showClient && (
                    <span>
                      <bdi>{ev.client.name}</bdi>{" "}
                      <span className="muted" dir="ltr">
                        {ev.client.host}
                      </span>
                    </span>
                  )}
                </div>
              )}
              {args && (
                <details className="event-args">
                  <summary>{t("ورودی‌ها", "Arguments")}</summary>
                  <pre dir="ltr">{args}</pre>
                </details>
              )}
            </li>
          );
        })}
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
