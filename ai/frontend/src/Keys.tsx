import { useCallback, useEffect, useState, type FormEvent } from "react";
import { api, ApiError, PAGE_SIZE, type ApiKey, type ApiRequest } from "./api";
import { fmtDate, fmtDateTime, fmtNum, fmtRelative } from "./format";
import { IconAlert, IconKey } from "./icons";
import { CopyButton } from "./Markdown";
import { RequestList } from "./Requests";

export const API_BASE_URL = "https://ai-api.salamlang.ir";
export const API_MODEL = "salam-ai";
const PLACEHOLDER = "sk-salam-...";
const HOURLY_LIMIT = 100;
const SAMPLE_PROMPT = "یک برنامه‌ی سلام بنویس که «سلام دنیا» را چاپ کند.";

type Example = { title: string; code: string };
type Tab = { id: string; label: string; install?: string; examples: Example[] };

function buildTabs(key: string): Tab[] {
  return [
    {
      id: "curl",
      label: "curl",
      examples: [
        {
          title: "ترمینال",
          code: `curl ${API_BASE_URL}/chat/completions \\
  -H "Authorization: Bearer ${key}" \\
  -H "Content-Type: application/json" \\
  -H "X-Salam-Project: my-project" \\
  -d '{
    "model": "${API_MODEL}",
    "messages": [
      {"role": "user", "content": "${SAMPLE_PROMPT}"}
    ]
  }'`,
        },
        {
          title: "پاسخ جریانی",
          code: `curl -N ${API_BASE_URL}/chat/completions \\
  -H "Authorization: Bearer ${key}" \\
  -H "Content-Type: application/json" \\
  -d '{
    "model": "${API_MODEL}",
    "stream": true,
    "messages": [
      {"role": "user", "content": "${SAMPLE_PROMPT}"}
    ]
  }'`,
        },
      ],
    },
    {
      id: "python",
      label: "Python",
      install: "pip install openai",
      examples: [
        {
          title: "Python",
          code: `from openai import OpenAI

client = OpenAI(
    base_url="${API_BASE_URL}",
    api_key="${key}",
)

response = client.chat.completions.create(
    model="${API_MODEL}",
    messages=[{"role": "user", "content": "${SAMPLE_PROMPT}"}],
    extra_headers={"X-Salam-Project": "my-project"},
)
print(response.choices[0].message.content)`,
        },
        {
          title: "پاسخ جریانی",
          code: `stream = client.chat.completions.create(
    model="${API_MODEL}",
    messages=[{"role": "user", "content": "${SAMPLE_PROMPT}"}],
    stream=True,
)
for chunk in stream:
    if chunk.choices:
        print(chunk.choices[0].delta.content or "", end="", flush=True)`,
        },
      ],
    },
    {
      id: "js",
      label: "JavaScript",
      install: "npm install openai",
      examples: [
        {
          title: "JavaScript",
          code: `import OpenAI from "openai";

const client = new OpenAI({
  baseURL: "${API_BASE_URL}",
  apiKey: "${key}",
  defaultHeaders: { "X-Salam-Project": "my-project" },
});

const response = await client.chat.completions.create({
  model: "${API_MODEL}",
  messages: [{ role: "user", content: "${SAMPLE_PROMPT}" }],
});
console.log(response.choices[0].message.content);`,
        },
        {
          title: "پاسخ جریانی",
          code: `const stream = await client.chat.completions.create({
  model: "${API_MODEL}",
  messages: [{ role: "user", content: "${SAMPLE_PROMPT}" }],
  stream: true,
});
for await (const chunk of stream) {
  process.stdout.write(chunk.choices[0]?.delta?.content ?? "");
}`,
        },
      ],
    },
  ];
}

function Code({ title, code }: Example) {
  return (
    <div className="code-block">
      <div className="code-head">
        <span className="code-lang">{title}</span>
        <CopyButton text={code} />
      </div>
      <pre dir="ltr">
        <code>{code}</code>
      </pre>
    </div>
  );
}

function Examples({ secret }: { secret: string }) {
  const [tab, setTab] = useState("curl");
  const tabs = buildTabs(secret || PLACEHOLDER);
  const current = tabs.find((t) => t.id === tab) || tabs[0];
  return (
    <section className="card panel">
      <h3>نمونه‌ی استفاده</h3>
      <div className="tabs examples-tabs" role="tablist" aria-label="زبان نمونه">
        {tabs.map((t) => (
          <button
            key={t.id}
            type="button"
            role="tab"
            id={`tab-${t.id}`}
            aria-selected={t.id === current.id}
            aria-controls={`panel-${t.id}`}
            className={t.id === current.id ? "active" : ""}
            onClick={() => setTab(t.id)}
            dir="ltr"
          >
            {t.label}
          </button>
        ))}
      </div>
      <div role="tabpanel" id={`panel-${current.id}`} aria-labelledby={`tab-${current.id}`}>
        {current.install && (
          <p className="muted small example-install">
            ابتدا کتابخانه‌ی رسمی OpenAI را نصب کنید: <code className="inline-code" dir="ltr">{current.install}</code>
          </p>
        )}
        {current.examples.map((ex) => (
          <Code key={ex.title} title={ex.title} code={ex.code} />
        ))}
      </div>
      <ul className="example-notes">
        <li>
          {secret ? "کلیدی که همین الان ساختید در نمونه‌ها گذاشته شده است." : (
            <>
              به جای <code className="inline-code" dir="ltr">{PLACEHOLDER}</code> کلید خودتان را بگذارید.
            </>
          )}{" "}
          بهتر است کلید را در متغیر محیطی نگه دارید و آن را در کد سمت مرورگر یا مخزن عمومی قرار ندهید.
        </li>
        <li>
          سرآیند اختیاری <code className="inline-code" dir="ltr">X-Salam-Project</code> نام پروژه را روی درخواست می‌گذارد تا در فهرست درخواست‌ها جدا دیده شود. اگر آن را نفرستید، نام کلید به جای نام پروژه ثبت
          می‌شود.
        </li>
        <li>
          هر حساب در هر ساعت حداکثر {fmtNum(HOURLY_LIMIT)} درخواست می‌تواند بفرستد. پس از آن، پاسخ با کد <code className="inline-code" dir="ltr">429</code> برمی‌گردد.
        </li>
      </ul>
    </section>
  );
}

function MyRequests() {
  const [requests, setRequests] = useState<ApiRequest[]>([]);
  const [loading, setLoading] = useState(true);
  const [hasMore, setHasMore] = useState(false);
  const [error, setError] = useState("");

  const load = useCallback(async (before?: number) => {
    setLoading(true);
    setError("");
    try {
      const r = await api.myRequests(before);
      setRequests((cur) => (before ? [...cur, ...r.requests] : r.requests));
      setHasMore(r.requests.length >= PAGE_SIZE);
    } catch (e) {
      setError(e instanceof ApiError ? e.message : "بارگذاری درخواست‌ها ناموفق بود.");
    } finally {
      setLoading(false);
    }
  }, []);

  useEffect(() => {
    load();
  }, [load]);

  return (
    <section className="card panel">
      <div className="panel-head">
        <h3>درخواست‌های من</h3>
        <button type="button" className="button ghost small" disabled={loading} onClick={() => load()}>
          تازه‌سازی
        </button>
      </div>
      {error && (
        <div className="form-error" role="alert">
          {error}
        </div>
      )}
      <RequestList
        requests={requests}
        loading={loading}
        hasMore={hasMore}
        onMore={() => load(requests[requests.length - 1]?.id)}
        empty="هنوز با کلیدهای شما درخواستی فرستاده نشده است."
      />
    </section>
  );
}

export function Keys({ notify }: { notify: (msg: string) => void }) {
  const [keys, setKeys] = useState<ApiKey[] | null>(null);
  const [error, setError] = useState("");
  const [name, setName] = useState("");
  const [creating, setCreating] = useState(false);
  const [formError, setFormError] = useState("");
  const [created, setCreated] = useState<{ key: ApiKey; secret: string } | null>(null);
  const [removing, setRemoving] = useState<number | null>(null);

  const load = useCallback(async () => {
    setError("");
    try {
      const r = await api.keys();
      setKeys(r.keys);
    } catch (e) {
      setError(e instanceof ApiError ? e.message : "بارگذاری کلیدها ناموفق بود.");
    }
  }, []);

  useEffect(() => {
    load();
  }, [load]);

  const create = async (e: FormEvent) => {
    e.preventDefault();
    const n = name.trim();
    if (!n) {
      setFormError("یک نام برای کلید بنویسید.");
      return;
    }
    setCreating(true);
    setFormError("");
    try {
      const r = await api.createKey(n);
      setCreated(r);
      setKeys((cur) => [r.key, ...(cur || [])]);
      setName("");
    } catch (err) {
      setFormError(err instanceof ApiError ? err.message : "ساخت کلید ناموفق بود.");
    } finally {
      setCreating(false);
    }
  };

  const revoke = async (k: ApiKey) => {
    if (!confirm(`کلید «${k.name}» باطل شود؟ برنامه‌هایی که از این کلید استفاده می‌کنند بلافاصله دیگر کار نخواهند کرد.`)) return;
    setRemoving(k.id);
    try {
      await api.revokeKey(k.id);
      setKeys((cur) => (cur ? cur.filter((x) => x.id !== k.id) : cur));
      if (created?.key.id === k.id) setCreated(null);
      notify(`کلید «${k.name}» باطل شد.`);
    } catch (e) {
      notify(e instanceof ApiError ? e.message : "باطل کردن کلید ناموفق بود.");
    } finally {
      setRemoving(null);
    }
  };

  return (
    <div className="page">
      <header className="page-head">
        <h2>کلیدهای API</h2>
        <p className="muted">
          دستیار سلام را در برنامه‌های خودتان به کار ببرید. این API با کلاینت‌های OpenAI سازگار است، پس کافی است نشانی پایه و مدل را عوض کنید و یک کلید شخصی بسازید.
        </p>
      </header>

      <section className="card panel">
        <h3>نشانی و مدل</h3>
        <dl className="api-facts">
          <div>
            <dt>نشانی پایه</dt>
            <dd>
              <code dir="ltr">{API_BASE_URL}</code>
              <CopyButton text={API_BASE_URL} />
            </dd>
          </div>
          <div>
            <dt>مدل</dt>
            <dd>
              <code dir="ltr">{API_MODEL}</code>
              <CopyButton text={API_MODEL} />
            </dd>
          </div>
        </dl>
      </section>

      <section className="card panel">
        <h3>ساخت کلید تازه</h3>
        <form className="key-form" onSubmit={create}>
          <label className="field">
            <span>نام کلید</span>
            <input value={name} onChange={(e) => setName(e.target.value)} maxLength={60} dir="auto" placeholder="مثلاً ربات تلگرام" autoComplete="off" />
          </label>
          <button type="submit" className="button primary" disabled={creating}>
            {creating ? <span className="spinner" /> : <IconKey width={18} height={18} />}
            ساخت کلید
          </button>
        </form>
        {formError && (
          <div className="form-error key-form-error" role="alert">
            {formError}
          </div>
        )}
        {created && (
          <div className="secret-box" role="status">
            <div className="secret-head">
              <IconAlert width={18} height={18} />
              <p>
                کلید «<bdi>{created.key.name}</bdi>» ساخته شد. همین حالا آن را رونوشت کنید و در جای امنی نگه دارید، چون دیگر نمایش داده نمی‌شود.
              </p>
            </div>
            <div className="secret-value">
              <code dir="ltr">{created.secret}</code>
              <CopyButton text={created.secret} label="رونوشت کلید" />
            </div>
            <button type="button" className="button ghost small" onClick={() => setCreated(null)}>
              کلید را ذخیره کردم، پنهانش کن
            </button>
          </div>
        )}
      </section>

      <section className="card panel">
        <div className="panel-head">
          <h3>کلیدهای فعال</h3>
        </div>
        {error && (
          <div className="form-error" role="alert">
            {error}{" "}
            <button type="button" className="link" onClick={load}>
              تلاش دوباره
            </button>
          </div>
        )}
        {keys === null && !error && (
          <div className="list-loading" role="status">
            <span className="spinner" />
            <span className="sr-only">در حال بارگذاری</span>
          </div>
        )}
        {keys && keys.length === 0 && <p className="empty-list">هنوز کلیدی نساخته‌اید. با فرم بالا اولین کلید را بسازید.</p>}
        {keys && keys.length > 0 && (
          <div className="table-wrap">
            <table className="data-table">
              <thead>
                <tr>
                  <th scope="col">نام</th>
                  <th scope="col">پیشوند</th>
                  <th scope="col">ساخته‌شده</th>
                  <th scope="col">آخرین استفاده</th>
                  <th scope="col">درخواست</th>
                  <th scope="col">
                    <span className="sr-only">کارها</span>
                  </th>
                </tr>
              </thead>
              <tbody>
                {keys.map((k) => (
                  <tr key={k.id}>
                    <td>
                      <bdi>{k.name}</bdi>
                    </td>
                    <td dir="ltr" className="cell-ltr cell-mono">
                      {k.prefix}…
                    </td>
                    <td title={fmtDateTime(k.created)}>{fmtDate(k.created)}</td>
                    <td title={k.last_used ? fmtDateTime(k.last_used) : undefined}>{fmtRelative(k.last_used)}</td>
                    <td>{fmtNum(k.requests)}</td>
                    <td>
                      <button type="button" className="button danger small" disabled={removing === k.id} onClick={() => revoke(k)} aria-label={`باطل کردن کلید ${k.name}`}>
                        {removing === k.id ? <span className="spinner" /> : "باطل کردن"}
                      </button>
                    </td>
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
        )}
      </section>

      <Examples secret={created?.secret || ""} />

      <MyRequests />
    </div>
  );
}
