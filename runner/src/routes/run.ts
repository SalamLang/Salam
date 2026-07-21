import type { Env } from "../env";
import { errMessage, log } from "../log";
import { badRequest, errorResponse, payloadTooLarge, relayResponse } from "../responses";
import { fetchSandbox } from "../sandbox-client";
import { BODY_TOO_LARGE_MESSAGE, MAX_BODY_BYTES, parseRunOptions, validateRunInput } from "../validation";

async function checkRateLimit(env: Env, clientIp: string, requestId: string): Promise<Response | null> {
  try {
    const result = await env.RUNNER_RATE_LIMITER.limit({ key: clientIp });
    if (!result.success) {
      log("info", "rate_limited", { requestId, clientIp });
      return errorResponse(429, "rate_limited", "too many requests - slow down and try again shortly", requestId);
    }
  } catch (err) {
    log("warn", "rate_limiter_error", { requestId, clientIp, error: errMessage(err) });
  }
  return null;
}

type SourceCodeResult = { code: string } | { error: Response };

async function readSourceCode(request: Request, requestId: string): Promise<SourceCodeResult> {
  const contentLength = Number(request.headers.get("content-length") ?? "0");
  if (contentLength > MAX_BODY_BYTES) {
    return { error: payloadTooLarge(BODY_TOO_LARGE_MESSAGE, requestId) };
  }

  let code: string;
  try {
    code = await request.text();
  } catch (err) {
    log("warn", "body_read_error", { requestId, error: errMessage(err) });
    return { error: badRequest("could not read request body", requestId) };
  }

  if (code.length > MAX_BODY_BYTES) {
    return { error: payloadTooLarge(BODY_TOO_LARGE_MESSAGE, requestId) };
  }
  return { code };
}

export async function handleRun(request: Request, env: Env, url: URL, requestId: string): Promise<Response> {
  const clientIp = request.headers.get("cf-connecting-ip") ?? "unknown";

  const rateLimitResponse = await checkRateLimit(env, clientIp, requestId);
  if (rateLimitResponse) return rateLimitResponse;

  const sourceResult = await readSourceCode(request, requestId);
  if ("error" in sourceResult) return sourceResult.error;
  const { code } = sourceResult;

  const { type, engine, language } = parseRunOptions(url);
  const validationError = validateRunInput({ code, type, engine, language });
  if (validationError) {
    return errorResponse(validationError.status, validationError.error, validationError.message, requestId);
  }

  log("info", "run_started", { requestId, clientIp, type, engine, language, codeBytes: code.length });
  const startedAt = Date.now();

  const sandboxResult = await fetchSandbox(
    env,
    requestId,
    "/run",
    {
      method: "POST",
      headers: { "content-type": "application/json", "x-request-id": requestId },
      body: JSON.stringify({ code, type, engine, language }),
    },
    requestId,
  );
  if (!sandboxResult.ok) return sandboxResult.errorResponse;

  log("info", "run_completed", {
    requestId,
    status: sandboxResult.response.status,
    durationMs: Date.now() - startedAt,
  });

  return relayResponse(sandboxResult.response, requestId);
}
