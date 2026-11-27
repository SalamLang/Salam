(() => {
  const version = new URL(self.location.href).searchParams.get("v");
  const compile = {};

  self.Module = {
    onRuntimeInitialized: () => {
      setTimeout(() => {
        const args = ["string", "string"];
        compile.app = Module.cwrap("salam_web_run_app", "string", args);
        compile.layout = Module.cwrap("salam_web_build_layout", "string", args);
        self.postMessage({ type: "ready" });
      }, 0);
    },
    onAbort: () => self.postMessage({ type: "crash" }),
  };

  self.addEventListener("message", (event) => {
    const { id, src, lang, mode } = event.data;
    try {
      const out = compile[mode](src, lang);
      const failed = Module._salam_web_last_failed() === 1;
      self.postMessage({ type: "done", id, out, failed });
    } catch (error) {
      self.postMessage({ type: "crash", id, out: String(error) });
    }
  });

  importScripts(`salam-wa-${version}.js`);
})();
