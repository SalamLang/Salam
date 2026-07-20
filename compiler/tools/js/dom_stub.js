// Minimal, deterministic DOM/BOM stub for running `salam js` bundles under
// plain Node (no browser, no jsdom). Just enough of document/window/storage
// for the tests/{en,fa,ar}/js/*.salam suite to exercise std/dom for real.

const allElements = [];

function normClass(el) {
  const c = el.attributes.get("class");
  return c ? c.split(/\s+/).filter(Boolean) : [];
}

function syncClassAttr(el, list) {
  if (list.length) el.attributes.set("class", list.join(" "));
  else el.attributes.delete("class");
}

class ClassList {
  constructor(el) {
    this.el = el;
  }
  add(name) {
    const l = normClass(this.el);
    if (!l.includes(name)) l.push(name);
    syncClassAttr(this.el, l);
  }
  remove(name) {
    syncClassAttr(
      this.el,
      normClass(this.el).filter((c) => c !== name),
    );
  }
  toggle(name) {
    const l = normClass(this.el);
    const i = l.indexOf(name);
    if (i >= 0) l.splice(i, 1);
    else l.push(name);
    syncClassAttr(this.el, l);
  }
  contains(name) {
    return normClass(this.el).includes(name);
  }
}

class StyleBag {
  constructor() {
    this.props = new Map();
  }
  setProperty(name, value) {
    this.props.set(name, String(value));
  }
  getPropertyValue(name) {
    return this.props.get(name) || "";
  }
}

class Element {
  constructor(tag) {
    this.tagName = String(tag || "").toUpperCase();
    this.attributes = new Map();
    this.classList = new ClassList(this);
    this.style = new StyleBag();
    this.children = [];
    this.parentNode = null;
    this._text = "";
    this._html = "";
    this.value = "";
    this.listeners = new Map();
    allElements.push(this);
  }
  get textContent() {
    return this._text;
  }
  set textContent(v) {
    this._text = String(v);
  }
  get innerHTML() {
    return this._html;
  }
  set innerHTML(v) {
    this._html = String(v);
  }
  setAttribute(name, value) {
    this.attributes.set(name, String(value));
  }
  getAttribute(name) {
    return this.attributes.has(name) ? this.attributes.get(name) : null;
  }
  appendChild(child) {
    child.parentNode = this;
    this.children.push(child);
    return child;
  }
  remove() {
    if (this.parentNode) {
      const i = this.parentNode.children.indexOf(this);
      if (i >= 0) this.parentNode.children.splice(i, 1);
      this.parentNode = null;
    }
  }
  addEventListener(type, handler) {
    if (!this.listeners.has(type)) this.listeners.set(type, []);
    this.listeners.get(type).push(handler);
  }
  removeEventListener(type, handler) {
    const l = this.listeners.get(type);
    if (!l) return;
    const i = l.indexOf(handler);
    if (i >= 0) l.splice(i, 1);
  }
  dispatchEvent(ev) {
    const l = this.listeners.get(ev.type);
    if (l) for (const h of l.slice()) h(ev);
  }
  _walk(cb) {
    cb(this);
    for (const c of this.children) c._walk(cb);
  }
}

function matches(el, sel) {
  sel = sel.trim();
  const m = /^([a-zA-Z][a-zA-Z0-9]*)?(?:#([\w-]+))?(?:\.([\w-]+))?$/.exec(sel);
  if (!m) return false;
  const [, tag, id, cls] = m;
  if (tag && el.tagName !== tag.toUpperCase()) return false;
  if (id && el.getAttribute("id") !== id) return false;
  if (cls && !el.classList.contains(cls)) return false;
  return true;
}

const documentElement = new Element("html");
documentElement.clientWidth = 1024;
documentElement.clientHeight = 768;
const body = new Element("body");
documentElement.appendChild(body);

const document = {
  readyState: "complete",
  title: "",
  body,
  documentElement,
  listeners: new Map(),
  createElement(tag) {
    return new Element(tag);
  },
  getElementById(id) {
    let found = null;
    documentElement._walk((el) => {
      if (!found && el.getAttribute("id") === id) found = el;
    });
    return found;
  },
  querySelector(sel) {
    let found = null;
    documentElement._walk((el) => {
      if (!found && matches(el, sel)) found = el;
    });
    return found;
  },
  querySelectorAll(sel) {
    const out = [];
    documentElement._walk((el) => {
      if (matches(el, sel)) out.push(el);
    });
    return out;
  },
  addEventListener(type, handler) {
    if (!this.listeners.has(type)) this.listeners.set(type, []);
    this.listeners.get(type).push(handler);
  },
};

const window = {
  innerWidth: 1280,
  innerHeight: 800,
  scrollX: 0,
  scrollY: 0,
  listeners: new Map(),
  addEventListener(type, handler) {
    if (!this.listeners.has(type)) this.listeners.set(type, []);
    this.listeners.get(type).push(handler);
  },
  removeEventListener(type, handler) {
    const l = this.listeners.get(type);
    if (!l) return;
    const i = l.indexOf(handler);
    if (i >= 0) l.splice(i, 1);
  },
  alert(msg) {
    console.log(`[alert] ${String(msg)}`);
  },
  confirm(_msg) {
    return true;
  },
  prompt(_msg) {
    return "stub-answer";
  },
};

const localStorage = (() => {
  const m = new Map();
  return {
    getItem: (k) => (m.has(k) ? m.get(k) : null),
    setItem: (k, v) => m.set(k, String(v)),
    removeItem: (k) => m.delete(k),
  };
})();

const performance = { now: () => 123 };

function requestAnimationFrame(_fn) {
  return 0;
}

global.document = document;
global.window = window;
global.localStorage = localStorage;
global.performance = performance;
global.requestAnimationFrame = requestAnimationFrame;
global.getComputedStyle = (el) => ({
  getPropertyValue: (name) => el.style.getPropertyValue(name),
});
global.Image = class {};

function fireAutoEvents() {
  for (const el of allElements) {
    const type = el.getAttribute("data-autofire");
    if (!type) continue;
    const ev = {
      type,
      target: el,
      key: el.getAttribute("data-autofire-key") || "",
      clientX: Number(el.getAttribute("data-autofire-x") || 0),
      clientY: Number(el.getAttribute("data-autofire-y") || 0),
      defaultPrevented: false,
      preventDefault() {
        this.defaultPrevented = true;
      },
      stopPropagation() {},
    };
    el.dispatchEvent(ev);
  }
  if (window.listeners.has("resize")) {
    const ev = { type: "resize" };
    for (const h of window.listeners.get("resize").slice()) h(ev);
  }
}

module.exports = { fireAutoEvents, document, window };
