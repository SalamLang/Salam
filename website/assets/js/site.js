(() => {
  var root = document.documentElement;

  function saveTheme(value) {
    try {
      localStorage.setItem("theme", value);
    } catch (e) {}
  }

  function currentTheme() {
    var explicit = root.getAttribute("data-theme");
    if (explicit) {
      return explicit;
    }
    return window.matchMedia &&
      window.matchMedia("(prefers-color-scheme: dark)").matches
      ? "dark"
      : "light";
  }

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
    const still =
      window.matchMedia &&
      window.matchMedia("(prefers-reduced-motion: reduce)").matches;
    let played = false;
    const KEY = "salamat-closed";

    const remember = (value) => {
      try {
        if (value) {
          localStorage.setItem(KEY, String(Date.now()));
        } else {
          localStorage.removeItem(KEY);
        }
      } catch (e) {}
    };
    const recentlyClosed = () => {
      try {
        const at = Number(localStorage.getItem(KEY) || 0);
        return at > 0 && Date.now() - at < 7 * 24 * 3600 * 1000;
      } catch (e) {
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

    const open = (focus) => {
      panel.hidden = false;
      teaser.hidden = true;
      salamat.classList.add("is-open");
      launcher.setAttribute("aria-expanded", "true");
      launcher.classList.add("is-seen");
      requestAnimationFrame(() => panel.classList.add("is-shown"));
      play();
      if (focus) {
        setTimeout(() => input.focus(), still ? 0 : 350);
      }
    };
    const close = () => {
      panel.classList.remove("is-shown");
      salamat.classList.remove("is-open");
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
        bit.style.left = x + "px";
        bit.style.top = y + "px";
        bit.style.setProperty("--dx", Math.cos(angle) * dist + "px");
        bit.style.setProperty("--dy", Math.sin(angle) * dist - 40 + "px");
        bit.style.setProperty(
          "--spin",
          Math.round(Math.random() * 360 - 180) + "deg",
        );
        document.body.appendChild(bit);
        setTimeout(() => bit.remove(), 1100);
      }
    };

    const ask = (text, from) => {
      const q = text.trim();
      if (!q) {
        input.focus();
        return;
      }
      const box = (from || form).getBoundingClientRect();
      burst(box.left + box.width / 2, box.top + box.height / 2);
      const url =
        salamat.getAttribute("data-ai").replace(/\/$/, "") +
        "/?q=" +
        encodeURIComponent(q) +
        "&lang=" +
        (document.documentElement.lang === "fa" ? "fa" : "en");
      window.open(url, "_blank", "noopener");
      input.value = "";
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

    setTimeout(
      () => {
        salamat.classList.add("is-ready");
        if (recentlyClosed()) {
          return;
        }
        if (
          window.matchMedia &&
          window.matchMedia("(max-width: 640px)").matches
        ) {
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
