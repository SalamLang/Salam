import { useCallback, useEffect, useState, type FormEvent, type ReactNode } from "react";
import { api, ApiError, PAGE_SIZE, type ApiKey, type ApiRequest } from "./api";
import { fmtDate, fmtDateTime, fmtNum, fmtRelative } from "./format";
import { IconAlert, IconKey } from "./icons";
import { CopyButton, useHighlighted } from "./Markdown";
import { RequestList } from "./Requests";
import { confirmDialog } from "./Confirm";
import { t } from "./i18n";

export const API_BASE_URL = "https://ai-api.salamlang.ir";
export const API_MODEL = "salam";
const PLACEHOLDER = "sk-salam-...";
const HOURLY_LIMIT = 100;
const SAMPLE_PROMPT = t("یک برنامه‌ی سلام بنویس که «سلام دنیا» را چاپ کند.", "Write a Salam program that prints hello world.");
const SALAM_KEY = "sk-salam-KEY";
const SHORT_PROMPT = t("یک برنامه‌ی کوتاه سلام بنویس.", "Write a short Salam program.");

const SALAM_CHAT = `import net.http
import encoding.json
import encoding.value
import encoding.encerr

func main:
    mut headers := http.NewHeaders()
    headers.put("Authorization", "Bearer sk-salam-KEY")
    headers.put("Content-Type", "application/json")
    body := \`{"model": "salam", "messages": [{"role": "user", "content": "${SHORT_PROMPT}"}]}\`
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
    body := \`{"model": "salam", "stream": true, "messages": [{"role": "user", "content": "${SHORT_PROMPT}"}]}\`
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
          title: t("ترمینال", "Terminal"),
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
          title: t("پاسخ جریانی", "Streaming"),
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
          title: t("پاسخ جریانی", "Streaming"),
          lang: "salam",
          code: SALAM_STREAM.replace(SALAM_KEY, key),
          note: t("کلاینت HTTP کتابخانه‌ی استاندارد سلام وقتی برمی‌گردد که جریان پاسخ تمام شده باشد. پس این برنامه همه‌ی تکه‌های جریانی را بعد از پایان پاسخ یکجا چاپ می‌کند، نه هم‌زمان با رسیدنشان.", "The HTTP client in Salam's standard library returns only once the response stream has finished. So this program prints all the streamed chunks together at the end, not as they arrive."),
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
          title: t("پاسخ جریانی", "Streaming"),
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
          title: t("پاسخ جریانی", "Streaming"),
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
      <h3>{t("نمونه‌ی استفاده", "Usage examples")}</h3>
      <div className="tabs examples-tabs" role="tablist" aria-label={t("زبان نمونه", "Example language")}>
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
            {t("ابتدا کتابخانه‌ی رسمی OpenAI را نصب کنید:", "First install the official OpenAI library:")} <code className="inline-code" dir="ltr">{current.install}</code>
          </p>
        )}
        {current.file && current.run && (
          <p className="muted small example-install">
            {t("هر نمونه کامل است. آن را در پرونده‌ی", "Each example is complete. Save it as")} <code className="inline-code" dir="ltr">{current.file}</code> {t("بگذارید و با", "and run it with")} <code className="inline-code" dir="ltr">{current.run}</code>{t(" اجرا کنید.", ".")}
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
          {secret ? t("کلیدی که همین الان ساختید در نمونه‌ها گذاشته شده است.", "The key you just created is already filled in.") : (
            <>
              {t("به جای", "Replace")} <code className="inline-code" dir="ltr">{PLACEHOLDER}</code> {t("کلید خودتان را بگذارید.", "with your own key.")}
            </>
          )}{" "}
          {t("بهتر است کلید را در متغیر محیطی نگه دارید و آن را در کد سمت مرورگر یا مخزن عمومی قرار ندهید.", "Keep the key in an environment variable, and never put it in browser code or a public repository.")}
        </li>
        <li>
          {t("سرآیند اختیاری", "The optional")} <code className="inline-code" dir="ltr">X-Salam-Project</code> {t("نام پروژه را روی درخواست می‌گذارد تا در فهرست درخواست‌ها جدا دیده شود. اگر آن را نفرستید، نام کلید به جای نام پروژه ثبت می‌شود.", "header tags the request with a project name, so it shows up separately in the request list. If you leave it out, the key name is recorded as the project.")}
        </li>
        <li>
          {t(`هر حساب در هر ساعت حداکثر ${fmtNum(HOURLY_LIMIT)} درخواست می‌تواند بفرستد. پس از آن، پاسخ با کد`, `Each account can send up to ${fmtNum(HOURLY_LIMIT)} requests per hour. After that, requests get a`)} <code className="inline-code" dir="ltr">429</code>{t(" برمی‌گردد.", " response.")}
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
      setError(e instanceof ApiError ? e.message : t("بارگذاری درخواست‌ها ناموفق بود.", "Could not load requests."));
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
        <h3>{t("درخواست‌های من", "My requests")}</h3>
        <button type="button" className="button ghost small" disabled={loading} onClick={() => load()}>
          {t("تازه‌سازی", "Refresh")}
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
        empty={t("هنوز با کلیدهای شما درخواستی فرستاده نشده است.", "No requests have been sent with your keys yet.")}
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
      setError(e instanceof ApiError ? e.message : t("بارگذاری کلیدها ناموفق بود.", "Could not load keys."));
    }
  }, []);

  useEffect(() => {
    load();
  }, [load]);

  const create = async (e: FormEvent) => {
    e.preventDefault();
    const n = name.trim();
    if (!n) {
      setFormError(t("یک نام برای کلید بنویسید.", "Enter a name for the key."));
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
      setFormError(err instanceof ApiError ? err.message : t("ساخت کلید ناموفق بود.", "Could not create the key."));
    } finally {
      setCreating(false);
    }
  };

  const revoke = async (k: ApiKey) => {
    const ok = await confirmDialog({
      title: t("باطل کردن کلید", "Revoke key"),
      subject: k.name,
      message: t("برنامه‌هایی که از این کلید استفاده می‌کنند بلافاصله دیگر کار نخواهند کرد.", "Apps that use this key will stop working right away."),
      confirmLabel: t("باطل کن", "Revoke"),
      icon: "alert",
    });
    if (!ok) return;
    setRemoving(k.id);
    try {
      await api.revokeKey(k.id);
      setKeys((cur) => (cur ? cur.filter((x) => x.id !== k.id) : cur));
      if (created?.key.id === k.id) setCreated(null);
      notify(t(`کلید «${k.name}» باطل شد.`, `Key "${k.name}" was revoked.`));
    } catch (e) {
      notify(e instanceof ApiError ? e.message : t("باطل کردن کلید ناموفق بود.", "Could not revoke the key."));
    } finally {
      setRemoving(null);
    }
  };

  return (
    <div className="page">
      <header className="page-head">
        <h2>{t("کلیدهای API", "API keys")}</h2>
        <p className="muted">
          {t("دستیار سلام را در برنامه‌های خودتان به کار ببرید. این API با کلاینت‌های OpenAI سازگار است، پس کافی است نشانی پایه و مدل را عوض کنید و یک کلید شخصی بسازید.", "Use Salam AI in your own apps. The API is compatible with OpenAI clients, so just change the base URL and model and create a personal key.")}
        </p>
      </header>

      <section className="card panel">
        <h3>{t("نشانی و مدل", "Base URL and model")}</h3>
        <dl className="api-facts">
          <div>
            <dt>{t("نشانی پایه", "Base URL")}</dt>
            <dd>
              <code dir="ltr">{API_BASE_URL}</code>
              <CopyButton text={API_BASE_URL} />
            </dd>
          </div>
          <div>
            <dt>{t("مدل", "Model")}</dt>
            <dd>
              <code dir="ltr">{API_MODEL}</code>
              <CopyButton text={API_MODEL} />
            </dd>
          </div>
        </dl>
      </section>

      <section className="card panel">
        <h3>{t("ساخت کلید تازه", "Create a new key")}</h3>
        <form className="key-form" onSubmit={create}>
          <label className="field">
            <span>{t("نام کلید", "Key name")}</span>
            <input value={name} onChange={(e) => setName(e.target.value)} maxLength={60} dir="auto" placeholder={t("مثلاً ربات تلگرام", "e.g. Telegram bot")} autoComplete="off" />
          </label>
          <button type="submit" className="button primary" disabled={creating}>
            {creating ? <span className="spinner" /> : <IconKey width={18} height={18} />}
            {t("ساخت کلید", "Create key")}
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
                {t("کلید «", "Key \"")}<bdi>{created.key.name}</bdi>{t("» ساخته شد. همین حالا آن را رونوشت کنید و در جای امنی نگه دارید، چون دیگر نمایش داده نمی‌شود.", "\" was created. Copy it now and keep it somewhere safe, because it will not be shown again.")}
              </p>
            </div>
            <div className="secret-value">
              <code dir="ltr">{created.secret}</code>
              <CopyButton text={created.secret} label={t("رونوشت کلید", "Copy key")} />
            </div>
            <button type="button" className="button ghost small" onClick={() => setCreated(null)}>
              {t("کلید را ذخیره کردم، پنهانش کن", "I saved the key, hide it")}
            </button>
          </div>
        )}
      </section>

      <section className="card panel">
        <div className="panel-head">
          <h3>{t("کلیدهای فعال", "Active keys")}</h3>
        </div>
        {error && (
          <div className="form-error" role="alert">
            {error}{" "}
            <button type="button" className="link" onClick={load}>
              {t("تلاش دوباره", "Try again")}
            </button>
          </div>
        )}
        {keys === null && !error && (
          <div className="list-loading" role="status">
            <span className="spinner" />
            <span className="sr-only">{t("در حال بارگذاری", "Loading")}</span>
          </div>
        )}
        {keys && keys.length === 0 && <p className="empty-list">{t("هنوز کلیدی نساخته‌اید. با فرم بالا اولین کلید را بسازید.", "You have no keys yet. Create your first one with the form above.")}</p>}
        {keys && keys.length > 0 && (
          <div className="table-wrap">
            <table className="data-table">
              <thead>
                <tr>
                  <th scope="col">{t("نام", "Name")}</th>
                  <th scope="col">{t("پیشوند", "Prefix")}</th>
                  <th scope="col">{t("ساخته‌شده", "Created")}</th>
                  <th scope="col">{t("آخرین استفاده", "Last used")}</th>
                  <th scope="col">{t("درخواست", "Requests")}</th>
                  <th scope="col">
                    <span className="sr-only">{t("کارها", "Actions")}</span>
                  </th>
                </tr>
              </thead>
              <tbody>
                {keys.map((k) => (
                  <tr key={k.id}>
                    <td>
                      <bdi>{k.name}</bdi>
                      {k.custom && (
                        <span className="cell-sub" title={t("مدیر رفتار این کلید را تنظیم کرده است.", "An admin has customized how this key behaves.")}>
                          {t("تنظیم ویژه", "Custom setup")}
                        </span>
                      )}
                    </td>
                    <td dir="ltr" className="cell-ltr cell-mono">
                      {k.prefix}…
                    </td>
                    <td title={fmtDateTime(k.created)}>{fmtDate(k.created)}</td>
                    <td title={k.last_used ? fmtDateTime(k.last_used) : undefined}>{fmtRelative(k.last_used)}</td>
                    <td>{fmtNum(k.requests)}</td>
                    <td>
                      <button type="button" className="button danger small" disabled={removing === k.id} onClick={() => revoke(k)} aria-label={t(`باطل کردن کلید ${k.name}`, `Revoke key ${k.name}`)}>
                        {removing === k.id ? <span className="spinner" /> : t("باطل کردن", "Revoke")}
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
