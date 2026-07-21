import type { Env } from "./env";
import { landingPage } from "./landing";
import { errMessage, log } from "./log";
import { CORS_HEADERS, errorResponse } from "./responses";
import { handleRun } from "./routes/run";
import { handleVersion } from "./routes/version";

export { SalamSandbox } from "./container";

export default {
  async fetch(request: Request, env: Env): Promise<Response> {
    const requestId = crypto.randomUUID();
    const url = new URL(request.url);

    try {
      if (request.method === "OPTIONS") {
        return new Response(null, { status: 204, headers: CORS_HEADERS });
      }

      if (url.pathname === "/" && request.method === "GET") {
        return landingPage();
      }

      if (url.pathname === "/v1/run" && request.method === "POST") {
        return await handleRun(request, env, url, requestId);
      }

      if (url.pathname === "/v1/version" && request.method === "GET") {
        return await handleVersion(env, requestId);
      }

      return errorResponse(404, "not_found", undefined, requestId);
    } catch (err) {
      log("error", "unhandled_exception", {
        requestId,
        path: url.pathname,
        method: request.method,
        error: errMessage(err),
        stack: err instanceof Error ? err.stack : undefined,
      });
      return errorResponse(500, "internal_error", "unexpected server error", requestId);
    }
  },
} satisfies ExportedHandler<Env>;
