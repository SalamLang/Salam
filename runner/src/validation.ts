export const MAX_CODE_BYTES = 64 * 1024;
export const MAX_BODY_BYTES = MAX_CODE_BYTES + 4 * 1024;
export const BODY_TOO_LARGE_MESSAGE = `body must be under ${MAX_BODY_BYTES} bytes`;

const ALLOWED_TYPES = new Set(["program", "layout"]);
const ALLOWED_ENGINES = new Set(["interp", "llvm"]);
const ALLOWED_LANGUAGES = new Set(["en", "fa", "ar"]);

export interface RunOptions {
  type: string;
  engine: string;
  language: string;
}

export function parseRunOptions(url: URL): RunOptions {
  return {
    type: url.searchParams.get("type") ?? "program",
    engine: url.searchParams.get("engine") ?? "interp",
    language: url.searchParams.get("language") ?? "en",
  };
}

export interface RunInput extends RunOptions {
  code: string;
}

export interface ValidationError {
  status: number;
  error: "invalid_request" | "payload_too_large";
  message: string;
}

function validateEnum(
  label: string,
  value: string,
  allowed: ReadonlySet<string>,
): string | undefined {
  if (allowed.has(value)) return undefined;
  return `'${label}' query param must be one of: ${[...allowed].join(", ")}`;
}

export function validateRunInput(input: RunInput): ValidationError | undefined {
  if (!input.code) {
    return {
      status: 400,
      error: "invalid_request",
      message: "request body must contain Salam source code",
    };
  }
  if (input.code.length > MAX_CODE_BYTES) {
    return {
      status: 413,
      error: "payload_too_large",
      message: `code must be under ${MAX_CODE_BYTES} bytes`,
    };
  }

  const enumError =
    validateEnum("type", input.type, ALLOWED_TYPES) ??
    validateEnum("engine", input.engine, ALLOWED_ENGINES) ??
    validateEnum("language", input.language, ALLOWED_LANGUAGES);
  if (enumError) {
    return { status: 400, error: "invalid_request", message: enumError };
  }

  return undefined;
}
