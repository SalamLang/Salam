import { errMessage, log } from "./log";

export const CORS_HEADERS: Record<string, string> = {
  "Access-Control-Allow-Origin": "*",
  "Access-Control-Allow-Methods": "GET, POST, OPTIONS",
  "Access-Control-Allow-Headers": "Content-Type",
  "Access-Control-Max-Age": "86400",
};

// Wraps text that is already-serialized JSON (e.g. relayed from the
// sandbox) without re-encoding it.
export function rawJsonResponse(text: string, status = 200): Response {
  return new Response(text, {
    status,
    headers: { "content-type": "application/json;charset=UTF-8", ...CORS_HEADERS },
  });
}

export function jsonResponse(body: unknown, status = 200): Response {
  return rawJsonResponse(JSON.stringify(body), status);
}

export function errorResponse(status: number, error: string, message?: string, requestId?: string): Response {
  return jsonResponse({ ok: false, error, message, ...(requestId ? { requestId } : {}) }, status);
}

export function badRequest(message: string, requestId: string): Response {
  return errorResponse(400, "invalid_request", message, requestId);
}

export function payloadTooLarge(message: string, requestId: string): Response {
  return errorResponse(413, "payload_too_large", message, requestId);
}

// Reads a sandbox Response's body and relays it verbatim with our standard
// JSON + CORS headers, converting a failed read into the same
// sandbox_unavailable shape used when the sandbox couldn't be reached at all.
export async function relayResponse(response: Response, requestId: string): Promise<Response> {
  try {
    const text = await response.text();
    return rawJsonResponse(text, response.status);
  } catch (err) {
    log("error", "sandbox_response_read_error", { requestId, error: errMessage(err) });
    return errorResponse(502, "sandbox_unavailable", "the sandbox response could not be read", requestId);
  }
}
