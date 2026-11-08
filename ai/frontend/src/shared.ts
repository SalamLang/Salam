import { getToken } from "./api";

export function sharedLoader<T>(fn: () => Promise<T>) {
  let value: T | null = null;
  let owner = "";
  let pending: Promise<T> | null = null;
  const set = (v: T) => {
    value = v;
    owner = getToken();
  };
  return {
    peek: (): T | null => (owner === getToken() ? value : null),
    set,
    load: (): Promise<T> => {
      if (pending) return pending;
      pending = fn()
        .then((v) => {
          set(v);
          return v;
        })
        .finally(() => {
          pending = null;
        });
      return pending;
    },
  };
}
