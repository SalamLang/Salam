export type LogLevel = "info" | "warn" | "error";

export function errMessage(err: unknown): string {
  return err instanceof Error ? err.message : String(err);
}

export function log(
  level: LogLevel,
  event: string,
  fields: Record<string, unknown> = {},
): void {
  const line = JSON.stringify({
    level,
    event,
    ts: new Date().toISOString(),
    ...fields,
  });
  if (level === "error") console.error(line);
  else if (level === "warn") console.warn(line);
  else console.log(line);
}
