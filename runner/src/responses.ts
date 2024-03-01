import { errMessage, log } from "./log";

export const CORS_HEADERS: Record<string, string> = {
  "Access-Control-Allow-Origin": "*",
  "Access-Control-Allow-Methods": "GET, POST, OPTIONS",
  "Access-Control-Allow-Headers": "Content-Type",
  "Access-Control-Max-Age": "86400",
};

export function rawJsonResponse(text: string, status = 200): Response {
  return new Response(text, {
    status,
    headers: {
      "content-type": "application/json;charset=UTF-8",
      ...CORS_HEADERS,
    },
  });
}

export function jsonResponse(body: unknown, status = 200): Response {
  return rawJsonResponse(JSON.stringify(body), status);
}

export function errorResponse(
  status: number,
  error: string,
  message?: string,
  requestId?: string,
): Response {
  return jsonResponse(
    { ok: false, error, message, ...(requestId ? { requestId } : {}) },
    status,
  );
}

export function badRequest(message: string, requestId: string): Response {
  return errorResponse(400, "invalid_request", message, requestId);
}

export function payloadTooLarge(message: string, requestId: string): Response {
  return errorResponse(413, "payload_too_large", message, requestId);
}

export async function relayResponse(
  response: Response,
  requestId: string,
): Promise<Response> {
  try {
    const text = await response.text();
    return rawJsonResponse(text, response.status);
  } catch (err) {
    log("error", "sandbox_response_read_error", {
      requestId,
      error: errMessage(err),
    });
    return errorResponse(
      502,
      "sandbox_unavailable",
      "the sandbox response could not be read",
      requestId,
    );
  }
}
