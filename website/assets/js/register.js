(() => {
  const form = document.querySelector(".reg-form");
  const places = window.IRAN_PLACES;
  if (!form || !places) {
    return;
  }

  const lang = form.dataset.lang === "en" ? "en" : "fa";
  const persian = lang === "fa";
  const card = form.closest(".reg-card");
  const done = card.querySelector(".reg-done");
  const alertBox = form.querySelector(":scope > .reg-alert");
  const submit = form.querySelector(".reg-submit");
  const submitText = submit.querySelector("span");
  const idleText = submitText.textContent;

  const message = (name) => form.getAttribute(`data-msg-${name}`) || "";
  const field = (name) => form.querySelector(`[data-field="${name}"]`);

  const latinDigits = (text) =>
    text.replace(/[۰-۹٠-٩]/g, (d) => {
      const code = d.charCodeAt(0);
      return String(code - (code >= 0x06f0 ? 0x06f0 : 0x0660));
    });
  const shown = (text) =>
    persian
      ? String(text).replace(/[0-9]/g, (d) =>
          String.fromCharCode(0x06f0 + Number(d)),
        )
      : String(text);
  const loose = (text) =>
    latinDigits(String(text))
      .toLowerCase()
      .replace(/[يى]/g, "ی")
      .replace(/ك/g, "ک")
      .replace(/[أإآ]/g, "ا")
      .replace(/[ةۀ]/g, "ه")
      .replace(/[\s‌‏ً-ٟ'-]/g, "");

  const setError = (name, text) => {
    const box = field(name);
    if (!box) {
      return;
    }
    const line = box.querySelector(".reg-error");
    line.textContent = text;
    line.hidden = !text;
    box.classList.toggle("is-invalid", Boolean(text));
    for (const control of box.querySelectorAll("input, textarea")) {
      if (text) {
        control.setAttribute("aria-invalid", "true");
      } else {
        control.removeAttribute("aria-invalid");
      }
    }
    const birth = box.closest(".reg-birth");
    if (birth) {
      const first = birth.querySelector(".is-invalid .reg-error");
      const note = birth.querySelector(".reg-birth-note");
      note.textContent = first ? first.textContent : "";
      note.hidden = !first;
    }
  };

  const combo = (name, onChange) => {
    const box = field(name);
    const input = box.querySelector("input");
    const list = box.querySelector(".reg-options");
    let items = [];
    let visible = [];
    let value = "";
    let active = -1;

    const chosen = () => items.find((item) => item.value === value);
    const isOpen = () => !list.hidden;

    const mark = (index) => {
      active = index;
      const rows = list.querySelectorAll('[role="option"]');
      rows.forEach((row, i) => {
        row.classList.toggle("is-active", i === active);
      });
      if (rows[active]) {
        input.setAttribute("aria-activedescendant", rows[active].id);
        const row = rows[active];
        if (row.offsetTop < list.scrollTop) {
          list.scrollTop = row.offsetTop;
        } else if (
          row.offsetTop + row.offsetHeight >
          list.scrollTop + list.clientHeight
        ) {
          list.scrollTop = row.offsetTop + row.offsetHeight - list.clientHeight;
        }
      } else {
        input.removeAttribute("aria-activedescendant");
      }
    };

    const draw = (query) => {
      const q = loose(query);
      const starts = [];
      const rest = [];
      for (const item of items) {
        const at = q ? item.keys.findIndex((k) => k.startsWith(q)) : 0;
        if (at >= 0) {
          starts.push(item);
        } else if (item.keys.some((k) => k.includes(q))) {
          rest.push(item);
        }
      }
      visible = starts.concat(rest);
      list.textContent = "";
      visible.forEach((item, i) => {
        const row = document.createElement("li");
        row.id = `${input.id}-option-${i}`;
        row.setAttribute("role", "option");
        row.setAttribute("aria-selected", String(item.value === value));
        const label = document.createElement("span");
        label.textContent = item.label;
        row.append(label);
        if (item.note) {
          const note = document.createElement("small");
          note.textContent = item.note;
          row.append(note);
        }
        row.addEventListener("pointermove", () => {
          if (active !== i) {
            mark(i);
          }
        });
        row.addEventListener("click", () => pick(item));
        list.append(row);
      });
      if (visible.length === 0) {
        const empty = document.createElement("li");
        empty.className = "reg-empty";
        empty.textContent = message("no-results");
        list.append(empty);
      }
    };

    const close = () => {
      list.hidden = true;
      box.classList.remove("is-open", "is-above");
      input.setAttribute("aria-expanded", "false");
      input.removeAttribute("aria-activedescendant");
    };

    const open = (query) => {
      if (input.disabled) {
        return;
      }
      draw(query);
      list.hidden = false;
      box.classList.add("is-open");
      input.setAttribute("aria-expanded", "true");
      const rect = input.getBoundingClientRect();
      const below = window.innerHeight - rect.bottom;
      box.classList.toggle(
        "is-above",
        below < list.offsetHeight + 12 && rect.top > below,
      );
      list.scrollTop = 0;
      const at = query ? 0 : visible.findIndex((item) => item.value === value);
      mark(visible.length ? Math.max(at, 0) : -1);
    };

    const pick = (item) => {
      const changed = value !== item.value;
      value = item.value;
      input.value = item.label;
      setError(name, "");
      close();
      if (document.activeElement === input) {
        input.select();
      }
      if (changed && onChange) {
        onChange(value);
      }
    };

    const clear = () => {
      const changed = value !== "";
      value = "";
      input.value = "";
      if (changed && onChange) {
        onChange(value);
      }
    };

    const settle = () => {
      const typed = loose(input.value);
      const current = chosen();
      if (!typed) {
        clear();
      } else if (!current || loose(current.label) !== typed) {
        const exact = items.find((item) => item.keys.includes(typed));
        if (exact) {
          pick(exact);
        } else {
          input.value = current ? current.label : "";
        }
      }
      close();
    };

    input.addEventListener("focus", () => {
      input.select();
      open("");
    });
    input.addEventListener("mousedown", (e) => {
      if (input.value && chosen()?.label === input.value) {
        e.preventDefault();
        input.focus();
        input.select();
      }
    });
    input.addEventListener("click", () => {
      if (!isOpen()) {
        open("");
      }
    });
    input.addEventListener("input", () => open(input.value));
    input.addEventListener("blur", settle);
    list.addEventListener("mousedown", (e) => e.preventDefault());
    input.addEventListener("keydown", (e) => {
      if (e.key === "ArrowDown" || e.key === "ArrowUp") {
        e.preventDefault();
        if (!isOpen()) {
          open("");
        } else if (visible.length) {
          const step = e.key === "ArrowDown" ? 1 : -1;
          mark((active + step + visible.length) % visible.length);
        }
      } else if (e.key === "Enter") {
        e.preventDefault();
        if (isOpen() && visible[active]) {
          pick(visible[active]);
        }
      } else if (e.key === "Escape" && isOpen()) {
        e.preventDefault();
        const current = chosen();
        input.value = current ? current.label : "";
        close();
      } else if (e.key === "Tab" && isOpen() && input.value) {
        const current = chosen();
        const typed = !current || current.label !== input.value;
        if (typed && visible[active]) {
          pick(visible[active]);
        }
      }
    });

    return {
      get value() {
        return value;
      },
      input,
      clear,
      fill(next) {
        items = next.map((item) =>
          Object.assign({}, item, {
            keys: [item.label, item.note || "", item.value]
              .filter(Boolean)
              .map(loose),
          }),
        );
        if (value && !chosen()) {
          clear();
        }
      },
      enable(on, placeholder) {
        input.disabled = !on;
        input.placeholder = placeholder;
        if (!on) {
          close();
        }
      },
    };
  };

  const jalali = persian;
  const thisYear = () => {
    const now = new Date();
    if (!jalali) {
      return now.getFullYear();
    }
    const month = now.getMonth() + 1;
    const beforeNewYear = month < 3 || (month === 3 && now.getDate() < 21);
    return now.getFullYear() - (beforeNewYear ? 622 : 621);
  };
  const monthLength = (year, month) => {
    if (!month) {
      return 31;
    }
    if (jalali) {
      if (month <= 6) {
        return 31;
      }
      if (month <= 11) {
        return 30;
      }
      return !year || [1, 5, 9, 13, 17, 22, 26, 30].includes(year % 33)
        ? 30
        : 29;
    }
    return new Date(year || 2000, month, 0).getDate();
  };
  const numbers = (from, to) => {
    const out = [];
    const step = from <= to ? 1 : -1;
    for (let n = from; n !== to + step; n += step) {
      out.push({ value: String(n), label: shown(n) });
    }
    return out;
  };

  let day = null;
  let month = null;
  let year = null;
  const fillDays = () => {
    day.fill(
      numbers(1, monthLength(Number(year?.value), Number(month?.value))),
    );
  };
  day = combo("birth_day");
  month = combo("birth_month", () => fillDays());
  year = combo("birth_year", () => fillDays());
  month.fill(
    form.dataset.months.split(",").map((label, i) => ({
      value: String(i + 1),
      label: label.trim(),
      note: shown(i + 1),
    })),
  );
  fillDays();

  const city = combo("city");
  const province = combo("province", (name) => {
    const row = places.find((p) => p[0] === name);
    city.clear();
    city.fill(row ? row[2].map((c) => ({ value: c, label: c })) : []);
    city.enable(Boolean(row), message(row ? "pick-city" : "city-first"));
    setError("city", "");
  });
  province.fill(
    places.map(([fa, en]) => ({
      value: fa,
      label: persian ? fa : en,
      note: persian ? "" : fa,
    })),
  );
  city.enable(false, message("city-first"));

  const role = () => form.elements.role.value;
  const applyRole = () => {
    const current = role();
    for (const el of form.querySelectorAll("[data-only]")) {
      el.hidden = el.dataset.only !== current;
    }
    const legend = form.querySelector(".reg-birth legend");
    legend.textContent = legend.getAttribute(`data-label-${current}`);
    form.classList.toggle("is-teacher", current === "teacher");
    const newest = thisYear() - (current === "teacher" ? 14 : 4);
    year.fill(numbers(newest, thisYear() - 90));
    for (const box of form.querySelectorAll("[data-field]")) {
      setError(box.dataset.field, "");
    }
    alertBox.hidden = true;
  };
  for (const radio of form.querySelectorAll('input[name="role"]')) {
    radio.addEventListener("change", applyRole);
  }
  const asked = new URLSearchParams(window.location.search).get("role");
  if (asked === "teacher" || asked === "student") {
    form.elements.role.value = asked;
  }
  applyRole();

  const mobile = form.elements.mobile;
  const cleanMobile = (text) => {
    let digits = latinDigits(text).replace(/[^0-9+]/g, "");
    digits = digits.replace(/^(\+98|0098)/, "0").replace(/\+/g, "");
    return digits.slice(0, 11);
  };
  const wrongPrefix = (text) =>
    text.length > 0 && !"09".startsWith(text.slice(0, 2));
  mobile.addEventListener("input", () => {
    const caret = mobile.selectionStart ?? mobile.value.length;
    const before = cleanMobile(mobile.value.slice(0, caret)).length;
    const raw = latinDigits(mobile.value).replace(/[\s()-]/g, "");
    if ("+98".startsWith(raw) || "0098".startsWith(raw)) {
      if (raw.startsWith("+") || raw.startsWith("00")) {
        return;
      }
    }
    mobile.value = cleanMobile(mobile.value);
    mobile.setSelectionRange(before, before);
    setError(
      "mobile",
      wrongPrefix(mobile.value) ? message("mobile-prefix") : "",
    );
  });

  const intro = form.elements.intro;
  const count = field("intro").querySelector(".reg-count");
  const recount = () => {
    count.textContent = `${shown(intro.value.length)} / ${shown(intro.maxLength)}`;
  };
  intro.addEventListener("input", recount);
  recount();

  form.addEventListener("input", (e) => {
    const box = e.target.closest("[data-field]");
    if (box && box.dataset.field !== "mobile") {
      setError(box.dataset.field, "");
    }
  });

  const read = () => {
    const data = {
      role: role(),
      first_name: form.elements.first_name.value.trim(),
      last_name: form.elements.last_name.value.trim(),
      province: province.value,
      city: city.value,
      mobile: cleanMobile(mobile.value),
      calendar: jalali ? "jalali" : "gregorian",
      birth_year: year.value,
      coding_experience: form.elements.coding_experience.value,
      lang,
      website: form.elements.website.value,
    };
    if (data.role === "student") {
      data.birth_month = month.value;
      data.birth_day = day.value;
    } else {
      data.email = form.elements.email.value.trim();
      data.teaching_experience = form.elements.teaching_experience.value;
      data.intro = intro.value.trim();
    }
    return data;
  };

  const problems = (data) => {
    const found = [];
    const need = (name, ok, text) => {
      if (!ok) {
        found.push([name, text]);
      }
    };
    const teacher = data.role === "teacher";
    for (const name of ["first_name", "last_name"]) {
      need(
        name,
        data[name].length >= 2 && !/[<>]/.test(data[name]),
        message(data[name] ? "name" : "required"),
      );
    }
    let mobileProblem = "mobile";
    if (!data.mobile) {
      mobileProblem = "required";
    } else if (!data.mobile.startsWith("09")) {
      mobileProblem = "mobile-prefix";
    }
    need("mobile", /^09[0-9]{9}$/.test(data.mobile), message(mobileProblem));
    if (teacher) {
      need(
        "email",
        /^[^\s@<>]+@[^\s@<>]+\.[^\s@<>]+$/.test(data.email),
        message(data.email ? "email" : "required"),
      );
    } else {
      need("birth_day", data.birth_day, message("birth"));
      need("birth_month", data.birth_month, message("birth"));
    }
    need("birth_year", data.birth_year, message("birth"));
    need("province", data.province, message("pick-province"));
    need("city", data.city, message("choose"));
    if (teacher) {
      need("teaching_experience", data.teaching_experience, message("choose"));
    }
    need("coding_experience", data.coding_experience, message("choose"));
    return found;
  };

  const reveal = (name) => {
    const box = field(name);
    const control = box?.querySelector("input, textarea");
    if (control) {
      box.scrollIntoView({ block: "center" });
      control.focus({ preventScroll: true });
    }
  };

  const fail = (text) => {
    alertBox.textContent = text;
    alertBox.hidden = false;
  };

  const busy = (on) => {
    submit.disabled = on;
    submit.classList.toggle("is-busy", on);
    submitText.textContent = on ? message("sending") : idleText;
  };

  const serverMessages = {
    first_name: "name",
    last_name: "name",
    mobile: "mobile",
    email: "email",
    birth_year: "birth",
    birth_month: "birth",
    birth_day: "birth",
    province: "pick-province",
    intro: "too-long",
  };

  form.addEventListener("submit", async (e) => {
    e.preventDefault();
    if (submit.disabled) {
      return;
    }
    alertBox.hidden = true;
    const data = read();
    const found = problems(data);
    for (const box of form.querySelectorAll("[data-field]")) {
      setError(box.dataset.field, "");
    }
    for (const [name, text] of found) {
      setError(name, text);
    }
    if (found.length) {
      reveal(found[0][0]);
      return;
    }
    busy(true);
    const stop = new AbortController();
    const timer = setTimeout(() => stop.abort(), 20000);
    try {
      const res = await fetch(`${form.dataset.api}/registrations`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(data),
        signal: stop.signal,
      });
      let reply = {};
      try {
        reply = await res.json();
      } catch (_e) {}
      if (res.status === 201) {
        form.hidden = true;
        done.hidden = false;
        done.focus({ preventScroll: true });
        card.scrollIntoView({ block: "start" });
      } else if (res.status === 409) {
        setError("mobile", message("exists"));
        fail(message("exists"));
      } else if (res.status === 429) {
        fail(message("slow"));
      } else if (res.status === 400 && field(reply.field)) {
        setError(reply.field, message(serverMessages[reply.field] || "choose"));
        reveal(reply.field);
      } else {
        fail(message("server"));
      }
    } catch (_e) {
      fail(message("offline"));
    } finally {
      clearTimeout(timer);
      busy(false);
    }
  });

  done.querySelector(".reg-again").addEventListener("click", () => {
    const kept = role();
    form.reset();
    form.elements.role.value = kept;
    for (const c of [day, month, year, province]) {
      c.clear();
    }
    recount();
    applyRole();
    done.hidden = true;
    form.hidden = false;
    form.elements.first_name.focus();
  });

  submit.disabled = false;
})();
