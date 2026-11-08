import { useEffect, useRef, useState } from "react";
import type { WorkLabel } from "./workStatus";

const MIN_CHANGE_MS = 1500;
const KEEP_SPECIFIC_MS = 4000;

function same(a: WorkLabel, b: WorkLabel): boolean {
  return a.text === b.text && a.subject === b.subject;
}

export function useWorkLabel(next: WorkLabel): WorkLabel {
  const [shown, setShown] = useState(next);
  const changedAt = useRef(Date.now());
  const specificAt = useRef(next.specific ? Date.now() : 0);

  useEffect(() => {
    if (next.specific) specificAt.current = Date.now();
    if (same(next, shown)) return;
    const now = Date.now();
    let wait = Math.max(0, changedAt.current + MIN_CHANGE_MS - now);
    if (!next.specific && shown.specific) wait = Math.max(wait, specificAt.current + KEEP_SPECIFIC_MS - now);
    const id = window.setTimeout(() => {
      changedAt.current = Date.now();
      setShown(next);
    }, wait);
    return () => window.clearTimeout(id);
  }, [next.text, next.subject, next.specific, shown.text, shown.subject, shown.specific]);

  return shown;
}
