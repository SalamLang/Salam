import { getContainer } from "@cloudflare/containers";
import type { Env } from "./env";
import { errMessage, log } from "./log";
import { errorResponse } from "./responses";

export type SandboxResult =
  | { ok: true; response: Response }
  | { ok: false; errorResponse: Response };

export async function fetchSandbox(
  env: Env,
  instanceId: string,
  path: string,
  init: RequestInit,
  requestId: string,
): Promise<SandboxResult> {
  const sandbox = getContainer(env.SALAM_SANDBOX, instanceId);
  try {
    const response = await sandbox.fetch(`http://sandbox${path}`, init);
    return { ok: true, response };
  } catch (err) {
    log("error", "sandbox_fetch_error", {
      requestId,
      path,
      error: errMessage(err),
    });
    return {
      ok: false,
      errorResponse: errorResponse(
        502,
        "sandbox_unavailable",
        "the sandbox instance could not be reached",
        requestId,
      ),
    };
  }
}
