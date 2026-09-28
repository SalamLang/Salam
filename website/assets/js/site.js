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
