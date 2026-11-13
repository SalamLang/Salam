(() => {
  var root = document.documentElement;

  function saveTheme(value) {
    try {
      localStorage.setItem("theme", value);
    } catch (_e) {}
  }

  function currentTheme() {
    var explicit = root.getAttribute("data-theme");
    if (explicit) {
      return explicit;
    }
    return window.matchMedia?.("(prefers-color-scheme: dark)").matches
      ? "dark"
      : "light";
  }

  window.addEventListener("message", (e) => {
    var d = e.data;
    if (!d || d.salamPreview !== 1 || typeof d.n !== "string") {
      return;
    }
    var frame = document.getElementById("pv-" + d.n);
    if (frame && e.source === frame.contentWindow) {
      frame.style.height = Math.min(Math.max(d.h, 56), 576) + "px";
    }
  });

  var themeButton = document.querySelector(".theme-toggle");
  if (themeButton) {
    themeButton.addEventListener("click", () => {
      var next = currentTheme() === "dark" ? "light" : "dark";
      root.setAttribute("data-theme", next);
      saveTheme(next);
    });
  }

  var navButton = document.querySelector(".nav-toggle");
  var nav = document.getElementById("site-nav");
  if (navButton && nav) {
    navButton.addEventListener("click", () => {
      var open = nav.classList.toggle("is-open");
      navButton.setAttribute("aria-expanded", open ? "true" : "false");
    });
  }

  function copyText(text) {
    if (navigator.clipboard && window.isSecureContext) {
      return navigator.clipboard.writeText(text);
    }
    return new Promise((resolve, reject) => {
      var area = document.createElement("textarea");
      area.value = text;
      area.setAttribute("readonly", "");
      area.style.position = "fixed";
      area.style.opacity = "0";
      document.body.appendChild(area);
      area.select();
      try {
        document.execCommand("copy");
        resolve();
      } catch (e) {
        reject(e);
      }
      document.body.removeChild(area);
    });
  }

  function salamatWidget(salamat) {
    const panel = salamat.querySelector(".salamat-panel");
    const launcher = salamat.querySelector(".salamat-launcher");
    const teaser = salamat.querySelector(".salamat-teaser");
    const closer = salamat.querySelector(".salamat-close");
    const form = salamat.querySelector(".salamat-form");
    const input = form.querySelector("input");
    const lines = salamat.querySelectorAll(".salamat-msg");
    const typing = salamat.querySelector(".salamat-typing");
    const chips = salamat.querySelector(".salamat-chips");
    const still = window.matchMedia?.(
      "(prefers-reduced-motion: reduce)",
    ).matches;
    let played = false;
    const KEY = "salamat-closed";

    const remember = (value) => {
      try {
        if (value) {
          localStorage.setItem(KEY, String(Date.now()));
        } else {
          localStorage.removeItem(KEY);
        }
      } catch (_e) {}
    };
    const recentlyClosed = () => {
      try {
        const at = Number(localStorage.getItem(KEY) || 0);
        return at > 0 && Date.now() - at < 7 * 24 * 3600 * 1000;
      } catch (_e) {
        return false;
      }
    };
    const wait = (ms) =>
      new Promise((done) => setTimeout(done, still ? 0 : ms));

    const play = async () => {
      if (played) {
        return;
      }
      played = true;
      for (let i = 0; i < lines.length; i++) {
        typing.hidden = false;
        await wait(i === 0 ? 650 : 900 + lines[i].textContent.length * 12);
        typing.hidden = true;
        lines[i].hidden = false;
        lines[i].classList.add("is-in");
        await wait(180);
      }
      chips.hidden = false;
      chips.classList.add("is-in");
    };

    const phone = () =>
      window.matchMedia && window.matchMedia("(max-width: 640px)").matches;
    const lockPage = (on) => {
      document.documentElement.classList.toggle("salamat-lock", on && phone());
    };
    const fit = () => {
      const vv = window.visualViewport;
      if (vv && phone() && salamat.classList.contains("is-open")) {
        panel.style.height = `${vv.height}px`;
        panel.style.top = `${vv.offsetTop}px`;
        body.scrollTop = body.scrollHeight;
      } else {
        panel.style.height = "";
        panel.style.top = "";
      }
    };
    if (window.visualViewport) {
      window.visualViewport.addEventListener("resize", fit);
      window.visualViewport.addEventListener("scroll", fit);
    }
    let pushed = false;
    window.addEventListener("popstate", () => {
      if (pushed && salamat.classList.contains("is-open")) {
        pushed = false;
        close();
      }
    });

    const settle = () => {
      const chatted = state.msgs.some((m) => m.role === "user");
      const place = () => {
        body.scrollTop = chatted ? body.scrollHeight : 0;
      };
      place();
      setTimeout(place, still ? 0 : 120);
      setTimeout(place, still ? 0 : 520);
    };

    const open = (focus) => {
      lockPage(true);
      if (phone() && !pushed) {
        history.pushState({ salamat: true }, "");
        pushed = true;
      }
      panel.hidden = false;
      teaser.hidden = true;
      salamat.classList.add("is-open");
      launcher.setAttribute("aria-expanded", "true");
      launcher.classList.add("is-seen");
      requestAnimationFrame(() => {
        panel.classList.add("is-shown");
        fit();
        settle();
      });
      play();
      if (focus && !phone()) {
        setTimeout(() => input.focus(), still ? 0 : 350);
      }
    };
    const close = () => {
      lockPage(false);
      if (pushed) {
        pushed = false;
        history.back();
      }
      panel.classList.remove("is-shown");
      salamat.classList.remove("is-open");
      fit();
      launcher.setAttribute("aria-expanded", "false");
      setTimeout(
        () => {
          if (!salamat.classList.contains("is-open")) {
            panel.hidden = true;
          }
        },
        still ? 0 : 260,
      );
      remember(true);
    };

    const burst = (x, y) => {
      if (still) {
        return;
      }
      const marks = ["👋", "✨", "💛", "🧡", "⭐", "🎉", "💻"];
      for (let i = 0; i < 18; i++) {
        const bit = document.createElement("span");
        bit.className = "salamat-spark";
        bit.textContent = marks[i % marks.length];
        const angle = (Math.PI * 2 * i) / 18 + Math.random() * 0.4;
        const dist = 70 + Math.random() * 90;
        bit.style.left = `${x}px`;
        bit.style.top = `${y}px`;
        bit.style.setProperty("--dx", `${Math.cos(angle) * dist}px`);
        bit.style.setProperty("--dy", `${Math.sin(angle) * dist - 40}px`);
        bit.style.setProperty(
          "--spin",
          `${Math.round(Math.random() * 360 - 180)}deg`,
        );
        document.body.appendChild(bit);
        setTimeout(() => bit.remove(), 1100);
      }
    };

    const lang = document.documentElement.lang === "fa" ? "fa" : "en";
    const aiUrl = salamat.getAttribute("data-ai").replace(/\/$/, "");
    const apiUrl = (salamat.getAttribute("data-api") || "").replace(/\/$/, "");
    const cta = salamat.querySelector(".salamat-cta");
    const count = salamat.querySelector(".salamat-count");
    const note = salamat.querySelector(".salamat-note");
    const send = form.querySelector("button");
    const thread = salamat.querySelector(".salamat-thread");
    const body = salamat.querySelector(".salamat-body");
    const STORE = "salamat-chat";
    const RTL = /[\u0590-\u08ff\ufb1d-\ufdff\ufe70-\ufeff]/;
    const LTR = /[A-Za-z]/;
    let state = { guest: "", msgs: [], used: 0, limit: 15, blocked: false };
    let busy = false;
    try {
      const saved = JSON.parse(localStorage.getItem(STORE) || "null");
      if (saved && Array.isArray(saved.msgs)) {
        state = Object.assign(state, saved);
      }
    } catch (_e) {}
    const persist = () => {
      try {
        localStorage.setItem(
          STORE,
          JSON.stringify(
            Object.assign({}, state, { msgs: state.msgs.slice(-40) }),
          ),
        );
      } catch (_e) {}
    };

    const dirOf = (text) => {
      for (const ch of text) {
        if (RTL.test(ch)) {
          return "rtl";
        }
        if (LTR.test(ch)) {
          return "ltr";
        }
      }
      return document.documentElement.dir || "ltr";
    };
    input.addEventListener("input", () => {
      input.dir = input.value
        ? dirOf(input.value)
        : document.documentElement.dir || "ltr";
    });

    const escapeHtml = (text) =>
      text.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;");
    const escapeAttr = (text) => escapeHtml(text).replace(/"/g, "&quot;");
    const bold = (text) =>
      escapeHtml(text).replace(/\*\*([^*\n]+)\*\*/g, "<strong>$1</strong>");
    const safeHref = (url) => /^(https?:\/\/|mailto:)[^\s<>"]+$/i.test(url);
    const anchor = (href, label) =>
      `<a href="${escapeAttr(href)}" target="_blank" rel="noopener noreferrer">${label}</a>`;
    const linkRe =
      /\[([^\]\n]+)\]\(\s*([^)\s]+)\s*\)|(?:https?:\/\/|mailto:)[A-Za-z0-9\-._~:/?#[\]@!$&'()*+,;=%]+/gi;
    const trimUrl = (url) => {
      let u = url;
      for (;;) {
        if (/[.,;:!?'\]]$/.test(u)) {
          u = u.slice(0, -1);
        } else if (
          u.endsWith(")") &&
          u.split("(").length < u.split(")").length
        ) {
          u = u.slice(0, -1);
        } else {
          return u;
        }
      }
    };
    const linkify = (text) => {
      let out = "";
      let at = 0;
      for (const m of text.matchAll(linkRe)) {
        out += bold(text.slice(at, m.index));
        if (m[1] !== undefined) {
          out += safeHref(m[2]) ? anchor(m[2], bold(m[1])) : bold(m[0]);
          at = m.index + m[0].length;
        } else {
          const url = trimUrl(m[0]);
          out += anchor(url, escapeHtml(url));
          at = m.index + url.length;
        }
      }
      return out + bold(text.slice(at));
    };
    const inline = (text) =>
      text
        .split(/(`[^`\n]+`)/)
        .map((part, i) =>
          i % 2
            ? `<code dir="ltr">${escapeHtml(part.slice(1, -1))}</code>`
            : linkify(part),
        )
        .join("");
    const words = new Map();
    for (const [attr, cls] of [
      ["data-kw", "k"],
      ["data-types", "t"],
      ["data-builtins", "b"],
    ]) {
      for (const w of (salamat.getAttribute(attr) || "").split(/\s+/)) {
        if (w) {
          words.set(w, cls);
        }
      }
    }
    const decls = new Map();
    for (const [attr, cls] of [
      ["data-declf", "f"],
      ["data-decltype", "t"],
    ]) {
      for (const w of (salamat.getAttribute(attr) || "").split(/\s+/)) {
        if (w) {
          decls.set(w, cls);
        }
      }
    }
    const DIGIT = /[0-9\u06f0-\u06f9\u0660-\u0669]/;
    const identCp = (ch) =>
      ch === "_" ||
      /[A-Za-z0-9]/.test(ch) ||
      (ch.codePointAt(0) >= 128 &&
        !"\u060c\u061b\u061f\u00ab\u00bb\u00a0".includes(ch));
    const span = (cls, text) =>
      `<span class="tok-${cls}">${escapeHtml(text)}</span>`;
    const highlightLine = (line) => {
      const cps = [...line];
      const n = cps.length;
      let out = "";
      let i = 0;
      let pending = "";
      while (i < n) {
        const ch = cps[i];
        let j = i + 1;
        if (!identCp(ch) && ch !== " " && ch !== "\t") {
          pending = "";
        }
        if (ch === "/" && cps[i + 1] === "/") {
          out += span("c", cps.slice(i).join(""));
          break;
        }
        if (ch === '"' || ch === "`" || ch === "\u00ab") {
          const close = ch === "\u00ab" ? "\u00bb" : ch;
          while (j < n && cps[j] !== close) {
            if (cps[j] === "\\") {
              j += 1;
            }
            j += 1;
          }
          j = Math.min(n, j + 1);
          out += span("s", cps.slice(i, j).join(""));
        } else if (DIGIT.test(ch)) {
          while (
            j < n &&
            (DIGIT.test(cps[j]) || cps[j] === "." || cps[j] === "_")
          ) {
            j += 1;
          }
          out += span("n", cps.slice(i, j).join(""));
        } else if (identCp(ch)) {
          while (j < n && identCp(cps[j])) {
            j += 1;
          }
          const word = cps.slice(i, j).join("");
          let k = j;
          while (k < n && (cps[k] === " " || cps[k] === "\t")) {
            k += 1;
          }
          if (pending) {
            out += span(pending, word);
            pending = "";
          } else if (words.has(word)) {
            out += span(words.get(word), word);
            pending = decls.get(word) || "";
          } else if (cps[k] === "(") {
            out += span("f", word);
          } else {
            out += escapeHtml(word);
          }
        } else {
          out += escapeHtml(ch);
        }
        i = j;
      }
      return out;
    };
    const SALAM = /^(salam|\u0633\u0644\u0627\u0645|slm)$/i;
    const SHELLS =
      /^(sh|bash|zsh|fish|shell|console|terminal|powershell|pwsh|ps1|cmd|bat|batch|dos)$/i;
    const strongDir = (line) => {
      for (const ch of line) {
        if (RTL.test(ch)) {
          return "rtl";
        }
        if (LTR.test(ch)) {
          return "ltr";
        }
      }
      return "";
    };
    const codeBlock = (lang, code) => {
      const shell = SHELLS.test(lang.trim());
      const colored =
        SALAM.test(lang.trim()) ||
        (!lang.trim() &&
          code.split(/[^\p{L}\p{N}_]+/u).some((w) => words.get(w) === "k"));
      const lines = code.replace(/\n$/, "").split("\n");
      let blockDir = "ltr";
      if (!shell) {
        for (const line of lines) {
          const d = strongDir(line);
          if (d) {
            blockDir = d;
            break;
          }
        }
      }
      let last = blockDir;
      const rows = lines.map((line) => {
        const d = shell ? "ltr" : strongDir(line) || last;
        last = d;
        const html = colored ? highlightLine(line) : escapeHtml(line);
        return `<span class="salamat-line" dir="${d}">${html || " "}</span>`;
      });
      return `<pre dir="${blockDir}"><code>${rows.join("")}</code></pre>`;
    };
    const render = (text) => {
      const out = [];
      const fence = /```([^\n`]*)\n([\s\S]*?)(?:```|$)/g;
      let at = 0;
      let m = fence.exec(text);
      const prose = (chunk) => {
        chunk
          .split(/\n{2,}/)
          .map((block) => block.trim())
          .filter(Boolean)
          .forEach((block) => {
            out.push(
              `<p dir="auto">${inline(block).replace(/\n/g, "<br>")}</p>`,
            );
          });
      };
      while (m) {
        prose(text.slice(at, m.index));
        out.push(codeBlock(m[1], m[2]));
        at = fence.lastIndex;
        if (m[0].length === 0) {
          break;
        }
        m = fence.exec(text);
      }
      prose(text.slice(at));
      return out.join("");
    };

    const bubble = (role, text) => {
      const li = document.createElement("li");
      li.className = `salamat-msg is-in${role === "user" ? " is-user" : ""}`;
      if (role === "user") {
        li.textContent = text;
        li.dir = dirOf(text);
      } else {
        li.innerHTML = render(text);
      }
      thread.insertBefore(li, typing);
      body.scrollTop = body.scrollHeight;
      return li;
    };

    const showCount = () => {
      if (state.used > 0) {
        const fmt = (v) => v.toLocaleString(lang === "fa" ? "fa-IR" : "en-US");
        count.textContent =
          fmt(state.used) +
          " / " +
          fmt(state.limit) +
          " " +
          count.getAttribute("data-label");
        count.hidden = false;
        note.hidden = true;
      }
    };
    const showCta = (question) => {
      const q = question ? `&q=${encodeURIComponent(question)}` : "";
      cta.querySelector(".salamat-cta-main").href =
        `${aiUrl}/?lang=${lang}&signup=1${q}`;
      cta.querySelector(".salamat-cta-alt").href = `${aiUrl}/?lang=${lang}${q}`;
      cta.hidden = false;
      cta.classList.add("is-in");
      chips.hidden = true;
      body.scrollTop = body.scrollHeight;
    };
    const lock = (on) => {
      busy = on;
      input.disabled = on || state.blocked;
      send.disabled = on || state.blocked;
    };

    const restore = () => {
      if (state.msgs.length === 0) {
        return;
      }
      played = true;
      lines.forEach((line) => {
        line.hidden = false;
      });
      state.msgs.forEach((m) => {
        bubble(m.role, m.text);
      });
      showCount();
      if (state.blocked) {
        showCta("");
        lock(false);
      } else {
        chips.hidden = true;
      }
    };

    const wait1 = (ms) => new Promise((done) => setTimeout(done, ms));
    const ours = (message) => {
      const e = new Error(message || "");
      e.ours = true;
      return e;
    };
    const failure = (e) => {
      if (e && e.ours && e.message) {
        return e.message;
      }
      const offline =
        e instanceof TypeError ||
        (e && e.offline) ||
        (typeof navigator !== "undefined" && navigator.onLine === false);
      return salamat.getAttribute(offline ? "data-offline" : "data-error");
    };
    const call = async (method, path, payload) => {
      const res = await fetch(apiUrl + path, {
        method,
        headers: Object.assign(
          { "X-Salam-Lang": lang },
          payload ? { "Content-Type": "application/json" } : {},
        ),
        body: payload ? JSON.stringify(payload) : undefined,
      });
      let data = {};
      try {
        data = await res.json();
      } catch (_e) {}
      return { status: res.status, data };
    };

    const phaseLabel = salamat.querySelector(".salamat-phase");
    const setPhase = (phase) => {
      const label = phase ? salamat.getAttribute(`data-phase-${phase}`) : "";
      phaseLabel.textContent = label || "";
      if (label && phase !== "writing") {
        typing.hidden = false;
        body.scrollTop = body.scrollHeight;
      }
    };
    const follow = async (id, show) => {
      if (!window.ReadableStream || !window.TextDecoder) {
        return false;
      }
      let res;
      try {
        res = await fetch(
          `${apiUrl}/guest/messages/${id}/stream?guest=${encodeURIComponent(state.guest)}&lang=${lang}`,
        );
      } catch (e) {
        return false;
      }
      if (!res.ok || !res.body) {
        return false;
      }
      const reader = res.body.getReader();
      const decoder = new TextDecoder();
      let buffer = "";
      let text1 = "";
      let finished = false;
      try {
        while (!finished) {
          const { value, done } = await reader.read();
          if (done) {
            break;
          }
          buffer += decoder.decode(value, { stream: true });
          let cut = buffer.indexOf("\n\n");
          while (cut >= 0) {
            const chunk = buffer.slice(0, cut);
            buffer = buffer.slice(cut + 2);
            cut = buffer.indexOf("\n\n");
            let name = "message";
            let data = "";
            for (const line of chunk.split("\n")) {
              if (line.startsWith("event: ")) {
                name = line.slice(7);
              } else if (line.startsWith("data: ")) {
                data += line.slice(6);
              }
            }
            if (!data) {
              continue;
            }
            let ev = {};
            try {
              ev = JSON.parse(data);
            } catch (e) {
              continue;
            }
            if (name === "delta") {
              text1 += ev.t || "";
              show(text1);
            } else if (name === "reset") {
              text1 = ev.t || "";
              show(text1);
            } else if (name === "phase") {
              setPhase(ev.phase);
            } else if (name === "done") {
              show(ev.body || "");
              finished = true;
            }
          }
        }
      } catch (e) {
        return finished;
      }
      return finished;
    };

    const ask = async (text, from) => {
      const q = text.trim();
      if (!q || busy) {
        input.focus();
        return;
      }
      if (state.blocked) {
        showCta(q);
        return;
      }
      const box = (from || form).getBoundingClientRect();
      burst(box.left + box.width / 2, box.top + box.height / 2);
      input.value = "";
      input.dir = document.documentElement.dir || "ltr";
      chips.hidden = true;
      bubble("user", q);
      state.msgs.push({ role: "user", text: q });
      persist();
      lock(true);
      typing.hidden = false;
      let answer = null;
      let shown = "";
      try {
        const sent = await call("POST", "/guest/messages", {
          guest: state.guest,
          text: q,
        });
        if (sent.data.error === "guest_limit") {
          typing.hidden = true;
          state.blocked = true;
          state.msgs.pop();
          persist();
          bubble("assistant", sent.data.message || "");
          showCta(q);
          lock(false);
          return;
        }
        if (sent.status !== 201) {
          throw ours(sent.data.message);
        }
        if (sent.data.guest) {
          state.guest = sent.data.guest;
        }
        state.used = sent.data.used || state.used + 1;
        state.limit = sent.data.limit || state.limit;
        showCount();
        persist();
        const show = (text1) => {
          if (!text1 || text1 === shown) {
            return;
          }
          typing.hidden = true;
          if (!answer) {
            answer = bubble("assistant", text1);
            answer.classList.add("is-live");
          } else {
            answer.innerHTML = render(text1);
            body.scrollTop = body.scrollHeight;
          }
          shown = text1;
        };
        const streamed = await follow(sent.data.id, show);
        if (!streamed) {
          const started = Date.now();
          let lost = 0;
          let bad = 0;
          while (Date.now() - started < 10 * 60 * 1000) {
            await wait1(1200);
            let got;
            try {
              got = await call(
                "GET",
                `/guest/messages/${sent.data.id}?guest=${encodeURIComponent(state.guest)}`,
              );
            } catch (_e) {
              lost += 1;
              if (lost >= 5) {
                const gone = new Error("");
                gone.offline = true;
                throw gone;
              }
              continue;
            }
            lost = 0;
            if (got.status !== 200) {
              bad += 1;
              if (bad >= 8) {
                throw ours("");
              }
              continue;
            }
            bad = 0;
            setPhase(got.data.phase);
            show(got.data.body || "");
            if (got.data.status !== "pending") {
              break;
            }
          }
        }
        if (answer) {
          answer.classList.remove("is-live");
        }
        setPhase("");
        typing.hidden = true;
        if (!shown) {
          shown = salamat.getAttribute("data-error");
          bubble("assistant", shown);
        }
        state.msgs.push({ role: "assistant", text: shown });
        if (state.used >= state.limit) {
          state.blocked = true;
          showCta("");
        }
        persist();
      } catch (e) {
        typing.hidden = true;
        setPhase("");
        if (answer) {
          answer.classList.remove("is-live");
        }
        bubble("assistant", failure(e));
      }
      lock(false);
      if (!state.blocked) {
        input.focus();
      }
    };

    launcher.addEventListener("click", () => {
      if (salamat.classList.contains("is-open")) {
        close();
      } else {
        remember(false);
        open(true);
      }
    });
    teaser.addEventListener("click", () => open(true));
    closer.addEventListener("click", close);
    document.addEventListener("keydown", (e) => {
      if (e.key === "Escape" && salamat.classList.contains("is-open")) {
        close();
        launcher.focus();
      }
    });
    form.addEventListener("submit", (e) => {
      e.preventDefault();
      ask(input.value, form);
    });
    chips.querySelectorAll("button").forEach((chip) => {
      chip.addEventListener("click", () => ask(chip.textContent, chip));
    });

    restore();

    setTimeout(
      () => {
        salamat.classList.add("is-ready");
        if (recentlyClosed()) {
          return;
        }
        if (window.matchMedia?.("(max-width: 640px)").matches) {
          teaser.hidden = false;
          requestAnimationFrame(() => teaser.classList.add("is-in"));
        } else {
          open(false);
        }
      },
      still ? 0 : 1200,
    );
  }

  const salamatRoot = document.querySelector(".salamat");
  if (salamatRoot) {
    salamatWidget(salamatRoot);
  }

  document.querySelectorAll("[data-copy-target]").forEach((button) => {
    var idle = button.textContent;
    button.addEventListener("click", () => {
      var target = document.getElementById(
        button.getAttribute("data-copy-target"),
      );
      if (!target) {
        return;
      }
      copyText(target.innerText.replace(/\s+$/, "")).then(() => {
        button.textContent = button.getAttribute("data-copied") || idle;
        setTimeout(() => {
          button.textContent = idle;
        }, 1600);
      });
    });
  });
})();

  function siteSearch() {
    const root = document.getElementById("site-search");
    const toggle = document.querySelector(".site-search-toggle");
    if (!root || !toggle) return;
    const input = document.getElementById("site-search-input");
    const results = root.querySelector(".site-search-results");
    const status = root.querySelector(".site-search-status");
    const closes = root.querySelectorAll("[data-search-close]");
    const indexUrl = document.body.getAttribute("data-search-index") || "/search-index.json";
    const lang = document.body.getAttribute("data-search-lang") || "en";
    const msgEmpty = document.body.getAttribute("data-search-empty") || "";
    const msgNone = document.body.getAttribute("data-search-none") || "No results found.";
    const msgLoading = document.body.getAttribute("data-search-loading") || "Loading...";
    let docs = null;
    let prepared = null;
    let loading = null;
    let active = -1;

    function norm(s) {
      if (!s) return "";
      let t = String(s).toLowerCase();
      t = t.replace(/[\u064B-\u065F\u0670\u0640]/g, "");
      t = t.replace(/[أإآٱ]/g, "ا").replace(/ى/g, "ی").replace(/ي/g, "ی").replace(/ئ/g, "ی");
      t = t.replace(/ك/g, "ک").replace(/ة/g, "ه").replace(/ؤ/g, "و");
      t = t.replace(/[۰-۹]/g, (d) => "0123456789"["۰۱۲۳۴۵۶۷۸۹".indexOf(d)]);
      return t;
    }

    function tokens(s) {
      const n = norm(s);
      const out = [];
      let cur = "";
      for (let i = 0; i < n.length; i++) {
        const c = n[i];
        const code = n.charCodeAt(i);
        const isWord = (code >= 48 && code <= 57) || (code >= 97 && code <= 122) || (code >= 0x0600 && code <= 0x06ff) || c === "_" || c === "-";
        if (isWord) cur += c;
        else if (cur) { if (cur.length > 1 || (cur >= "0" && cur <= "9")) out.push(cur); cur = ""; }
      }
      if (cur && (cur.length > 1 || (cur >= "0" && cur <= "9"))) out.push(cur);
      return out;
    }

    function prepare() {
      if (prepared || !docs) return;
      prepared = docs.map((d, i) => {
        const title = d.t || "";
        const sec = d.s || "";
        const text = d.x || "";
        const hay = norm(title + "\n" + sec + "\n" + text);
        const tt = tokens(title);
        const st = tokens(sec);
        return { i, d, hay, tt, st };
      });
    }

    async function ensureIndex() {
      if (docs) return true;
      if (loading) return loading;
      status.textContent = msgLoading;
      loading = fetch(indexUrl, { credentials: "same-origin" })
        .then((r) => { if (!r.ok) throw new Error("search index"); return r.json(); })
        .then((j) => { docs = Array.isArray(j) ? j : (j && j.docs) || []; prepare(); status.textContent = ""; return true; })
        .catch(() => { status.textContent = msgNone; return false; })
        .finally(() => { loading = null; });
      return loading;
    }

    function scoreDoc(p, qn, qts) {
      let score = 0;
      const d = p.d;
      const titleN = norm(d.t || "");
      if (titleN === qn) score += 120;
      else if (titleN.indexOf(qn) !== -1) score += 60;
      for (let i = 0; i < qts.length; i++) {
        const qt = qts[i];
        if (!qt) continue;
        let matched = false;
        for (let j = 0; j < p.tt.length; j++) {
          const tt = p.tt[j];
          if (tt === qt) { score += 20; matched = true; }
          else if (tt.indexOf(qt) === 0) { score += 12; matched = true; }
          else if (tt.indexOf(qt) !== -1) { score += 6; matched = true; }
        }
        for (let j = 0; j < p.st.length; j++) {
          const st = p.st[j];
          if (st === qt) { score += 10; matched = true; }
          else if (st.indexOf(qt) === 0) { score += 6; matched = true; }
          else if (st.indexOf(qt) !== -1) { score += 3; matched = true; }
        }
        if (!matched) {
          const pos = p.hay.indexOf(qt);
          if (pos === -1) return -1;
          score += 2;
          if (pos < 120) score += 1;
        }
      }
      if (d.k === "page") score += 2;
      if (d.k === "learn") score += 1;
      if (d.k === "std") score += 1;
      return score;
    }

    function esc(s) {
      return String(s || "").replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;").replace(/"/g, "&quot;");
    }

    function snippet(text, qts) {
      const s = String(text || "").replace(/\s+/g, " ").trim();
      if (!s) return "";
      let pos = -1;
      const sn = norm(s);
      for (let i = 0; i < qts.length; i++) {
        const p = sn.indexOf(qts[i]);
        if (p !== -1 && (pos === -1 || p < pos)) pos = p;
      }
      let start = pos > 40 ? pos - 40 : 0;
      let end = Math.min(s.length, start + 170);
      if (start > 0) start = s.indexOf(" ", start) + 1 || start;
      let out = (start > 0 ? "… " : "") + s.slice(start, end) + (end < s.length ? " …" : "");
      return esc(out);
    }

    function renderEmpty(msg) {
      results.innerHTML = '<li class="site-search-empty">' + esc(msg) + '</li>';
      active = -1;
    }

    function search(q) {
      const qn = norm(q).trim();
      if (!qn) { renderEmpty(msgEmpty); status.textContent = ""; return; }
      if (!prepared) { renderEmpty(msgLoading); return; }
      const qts = tokens(qn);
      if (!qts.length) { renderEmpty(msgEmpty); return; }
      const found = [];
      for (let i = 0; i < prepared.length; i++) {
        const sc = scoreDoc(prepared[i], qn, qts);
        if (sc >= 0) found.push({ p: prepared[i], sc });
      }
      found.sort((a, b) => b.sc - a.sc || (a.p.d.t || "").length - (b.p.d.t || "").length);
      const top = found.slice(0, 20);
      status.textContent = top.length ? "" : msgNone;
      if (!top.length) { renderEmpty(msgNone); return; }
      let html = "";
      for (let i = 0; i < top.length; i++) {
        const d = top[i].p.d;
        const sn = snippet(d.x || d.t || "", qts);
        html += '<li class="site-search-item" role="option" id="sr-' + i + '" aria-selected="false">' +
          '<a href="' + esc(d.u || "#") + '">' +
          '<span class="site-search-item-title">' + esc(d.t || "") + '</span>' +
          '<span class="site-search-item-meta">' + (d.s ? '<span class="site-search-item-tag">' + esc(d.s) + '</span>' : '') + '</span>' +
          (sn ? '<span class="site-search-item-snippet">' + sn + '</span>' : '') +
          '</a></li>';
      }
      results.innerHTML = html;
      active = top.length ? 0 : -1;
      updateActive();
    }

    function updateActive() {
      const items = results.querySelectorAll(".site-search-item");
      for (let i = 0; i < items.length; i++) {
        const sel = i === active;
        items[i].setAttribute("aria-selected", sel ? "true" : "false");
      }
      if (active >= 0 && items[active]) items[active].scrollIntoView({ block: "nearest" });
    }

    function openSearch() {
      root.hidden = false;
      document.documentElement.classList.add("search-lock");
      toggle.setAttribute("aria-expanded", "true");
      renderEmpty(msgEmpty);
      ensureIndex().then((ok) => {
        if (!ok) return;
        if (input.value.trim()) search(input.value);
        else status.textContent = "";
      });
      setTimeout(() => input && input.focus(), 10);
    }

    function closeSearch() {
      root.hidden = true;
      document.documentElement.classList.remove("search-lock");
      toggle.setAttribute("aria-expanded", "false");
      active = -1;
    }

    toggle.addEventListener("click", () => {
      if (root.hidden) openSearch(); else closeSearch();
    });
    for (let i = 0; i < closes.length; i++) closes[i].addEventListener("click", closeSearch);
    root.addEventListener("click", (e) => { if (e.target === root) closeSearch(); });

    let timer = null;
    input.addEventListener("input", () => {
      clearTimeout(timer);
      timer = setTimeout(() => search(input.value), 30);
    });

    document.addEventListener("keydown", (e) => {
      if ((e.ctrlKey || e.metaKey) && (e.key === "k" || e.key === "K")) {
        e.preventDefault();
        if (root.hidden) openSearch(); else closeSearch();
        return;
      }
      if (e.key === "/" && root.hidden && !e.ctrlKey && !e.metaKey && !e.altKey) {
        const tag = (document.activeElement && document.activeElement.tagName) || "";
        if (tag !== "INPUT" && tag !== "TEXTAREA" && !(document.activeElement && document.activeElement.isContentEditable)) {
          e.preventDefault();
          openSearch();
          return;
        }
      }
      if (root.hidden) return;
      const items = results.querySelectorAll(".site-search-item");
      if (e.key === "Escape") { e.preventDefault(); closeSearch(); toggle.focus(); }
      else if (e.key === "ArrowDown") { e.preventDefault(); if (items.length) { active = Math.min(active + 1, items.length - 1); updateActive(); } }
      else if (e.key === "ArrowUp") { e.preventDefault(); if (items.length) { active = Math.max(active - 1, 0); updateActive(); } }
      else if (e.key === "Enter" && active >= 0 && items[active]) {
        const a = items[active].querySelector("a");
        if (a) { e.preventDefault(); a.click(); }
      }
    });
  }

  siteSearch();
