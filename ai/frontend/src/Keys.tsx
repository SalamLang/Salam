import { useCallback, useEffect, useState, type FormEvent, type ReactNode } from "react";
import { api, ApiError, PAGE_SIZE, type ApiKey, type ApiRequest } from "./api";
import { fmtDate, fmtDateTime, fmtNum, fmtRelative } from "./format";
import { IconAlert, IconKey } from "./icons";
import { CopyButton, useHighlighted } from "./Markdown";
import { RequestList } from "./Requests";

export const API_BASE_URL = "https://ai-api.salamlang.ir";
export const API_MODEL = "salam";
const PLACEHOLDER = "sk-salam-...";
const HOURLY_LIMIT = 100;
const SAMPLE_PROMPT = "یک برنامه‌ی سلام بنویس که «سلام دنیا» را چاپ کند.";
const SALAM_KEY = "sk-salam-KEY";

const SALAM_CHAT = `import net.http
import encoding.json
import encoding.value
import encoding.encerr

func main:
    mut headers := http.NewHeaders()
    headers.put("Authorization", "Bearer sk-salam-KEY")
    headers.put("Content-Type", "application/json")
    body := \`{"model": "salam", "messages": [{"role": "user", "content": "یک برنامه‌ی کوتاه سلام بنویس."}]}\`
    r := http.PostWith("https://ai-api.salamlang.ir/chat/completions", body, headers)
    if http.Status(r) != 200:
        println "error", http.Status(r), http.Body(r)
        ret 1
    end
    mut err := encerr.EncodingError {}
    reply := json.Decode(http.Body(r), err)
    choice := value.ArrayGet(value.ObjectGet(reply, "choices"), 0)
    println value.AsString(value.ObjectGet(value.ObjectGet(choice, "message"), "content"))
    ret 0
end`;

const SALAM_STREAM = `import net.http
import encoding.json
import encoding.value
import encoding.encerr

func main:
    mut headers := http.NewHeaders()
    headers.put("Authorization", "Bearer sk-salam-KEY")
    headers.put("Content-Type", "application/json")
    body := \`{"model": "salam", "stream": true, "messages": [{"role": "user", "content": "یک برنامه‌ی کوتاه سلام بنویس."}]}\`
    r := http.PostWith("https://ai-api.salamlang.ir/chat/completions", body, headers)
    if http.Status(r) != 200:
        println "error", http.Status(r), http.Body(r)
        ret 1
    end
    each line in http.Body(r).split("\\n"):
        if not line.starts_with("data: ") or line == "data: [DONE]":
            continue
        end
        mut err := encerr.EncodingError {}
        chunk := json.Decode(line.substr(6, line.len() - 6), err)
        choices := value.ObjectGet(chunk, "choices")
        if value.ArrayLen(choices) > 0:
            delta := value.ObjectGet(value.ArrayGet(choices, 0), "delta")
            if value.ObjectHas(delta, "content"):
                print value.AsString(value.ObjectGet(delta, "content"))
            end
        end
    end
    println ""
    ret 0
end`;

type Example = { title: string; lang: string; code: string; note?: ReactNode };
type Tab = { id: string; label: string; install?: string; file?: string; run?: string; examples: Example[] };

function buildTabs(key: string): Tab[] {
  const pyClient = `from openai import OpenAI

client = OpenAI(
    base_url="${API_BASE_URL}",
    api_key="${key}",
)`;
  const jsClient = `import OpenAI from "openai";

const client = new OpenAI({
  baseURL: "${API_BASE_URL}",
  apiKey: "${key}",
  defaultHeaders: { "X-Salam-Project": "my-project" },
});`;
  return [
    {
      id: "curl",
      label: "curl",
      examples: [
        {
          title: "ترمینال",
          lang: "bash",
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
          lang: "bash",
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
      id: "salam",
      label: "Salam",
      file: "chat.salam",
      run: "salam run chat.salam",
      examples: [
        { title: "Salam", lang: "salam", code: SALAM_CHAT.replace(SALAM_KEY, key) },
        {
          title: "پاسخ جریانی",
          lang: "salam",
          code: SALAM_STREAM.replace(SALAM_KEY, key),
          note: "کلاینت HTTP کتابخانه‌ی استاندارد سلام وقتی برمی‌گردد که جریان پاسخ تمام شده باشد. پس این برنامه همه‌ی تکه‌های جریانی را بعد از پایان پاسخ یکجا چاپ می‌کند، نه هم‌زمان با رسیدنشان.",
        },
      ],
    },
    {
      id: "python",
      label: "Python",
      install: "pip install openai",
      file: "chat.py",
      run: "python chat.py",
      examples: [
        {
          title: "Python",
          lang: "python",
          code: `${pyClient}

response = client.chat.completions.create(
    model="${API_MODEL}",
    messages=[{"role": "user", "content": "${SAMPLE_PROMPT}"}],
    extra_headers={"X-Salam-Project": "my-project"},
)
print(response.choices[0].message.content)`,
        },
        {
          title: "پاسخ جریانی",
          lang: "python",
          code: `${pyClient}

stream = client.chat.completions.create(
    model="${API_MODEL}",
    messages=[{"role": "user", "content": "${SAMPLE_PROMPT}"}],
    stream=True,
)
for chunk in stream:
    if chunk.choices:
        print(chunk.choices[0].delta.content or "", end="", flush=True)
print()`,
        },
      ],
    },
    {
      id: "js",
      label: "JavaScript",
      install: "npm install openai",
      file: "chat.mjs",
      run: "node chat.mjs",
      examples: [
        {
          title: "JavaScript",
          lang: "javascript",
          code: `${jsClient}

const response = await client.chat.completions.create({
  model: "${API_MODEL}",
  messages: [{ role: "user", content: "${SAMPLE_PROMPT}" }],
});
console.log(response.choices[0].message.content);`,
        },
        {
          title: "پاسخ جریانی",
          lang: "javascript",
          code: `${jsClient}

const stream = await client.chat.completions.create({
  model: "${API_MODEL}",
  messages: [{ role: "user", content: "${SAMPLE_PROMPT}" }],
  stream: true,
});
for await (const chunk of stream) {
  process.stdout.write(chunk.choices[0]?.delta?.content ?? "");
}
process.stdout.write("\\n");`,
        },
      ],
    },
  ];
}

function Code({ title, lang, code }: Example) {
  const html = useHighlighted(code, lang);
  return (
    <div className="code-block">
      <div className="code-head">
        <span className="code-lang">{title}</span>
        <CopyButton text={code} />
      </div>
      <pre dir="ltr">
        <code dangerouslySetInnerHTML={{ __html: html }} />
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
        {current.file && current.run && (
          <p className="muted small example-install">
            هر نمونه کامل است. آن را در پرونده‌ی <code className="inline-code" dir="ltr">{current.file}</code> بگذارید و با <code className="inline-code" dir="ltr">{current.run}</code> اجرا کنید.
          </p>
        )}
        {current.examples.map((ex) => (
          <div key={ex.title}>
            <Code title={ex.title} lang={ex.lang} code={ex.code} />
            {ex.note && <p className="muted small example-note">{ex.note}</p>}
          </div>
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
