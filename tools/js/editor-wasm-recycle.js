(() => {
  const LIMIT = 536870912;
  const wrap = Module.cwrap;
  let fresh = null;

  function recycle() {
    const used = _sbrk(0) >>> 0;
    if (fresh === null) {
      fresh = HEAPU8.slice(0, used);
    } else if (used > LIMIT) {
      HEAPU8.set(fresh);
      HEAPU8.fill(0, fresh.length, used);
    }
  }

  Module.cwrap = (...spec) => {
    const call = wrap(...spec);
    return (...args) => {
      recycle();
      return call(...args);
    };
  };
})();
