import type { Env } from "../env";
import { relayResponse } from "../responses";
import { fetchSandbox } from "../sandbox-client";

const VERSION_PROBE_INSTANCE_ID = "version-probe";

export async function handleVersion(env: Env, requestId: string): Promise<Response> {
  const sandboxResult = await fetchSandbox(
    env,
    VERSION_PROBE_INSTANCE_ID,
    "/version",
    { headers: { "x-request-id": requestId } },
    requestId,
  );
  if (!sandboxResult.ok) return sandboxResult.errorResponse;
  return relayResponse(sandboxResult.response, requestId);
}
