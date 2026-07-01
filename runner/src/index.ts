// Add "ExecutionContext" to your import list
import type { ExecutionContext } from "@cloudflare/workers-types";

export type Env = {};

export default {
  async fetch(
    request: Request,
    env: Env,
    ctx: ExecutionContext,
  ): Promise<Response> {
    // 1. Enforce strict POST method restriction
    if (request.method !== "POST") {
      return new Response("Method Not Allowed", { status: 405 });
    }

    // 2. Protect against DoS by checking the Content-Length header first
    const contentLengthHeader = request.headers.get("content-length");
    const contentLength = contentLengthHeader
      ? parseInt(contentLengthHeader, 10)
      : NaN;
    const MAX_ALLOWED_SIZE = 100 * 1024; // Limit to 100 KB

    if (
      contentLengthHeader !== null &&
      (isNaN(contentLength) ||
        contentLength < 0 ||
        contentLength > MAX_ALLOWED_SIZE)
    ) {
      return new Response("Payload Too Large", { status: 413 });
    }

    try {
      // 3. Read the body safely
      const bodyText = await request.text();
      if (bodyText.length > MAX_ALLOWED_SIZE) {
        return new Response("Payload Too Large", { status: 413 });
      }

      // 4. Truncate input before logging to prevent log flooding
      const safeLogSnippet = bodyText.substring(0, 500);
      console.log(`[SECURE LOG] Received POST data snippet: ${safeLogSnippet}`);

      // 5. Sanitise or safely return the response
      return new Response(`Successfully processed request.`, {
        status: 200,
        headers: { "Content-Type": "text/plain" },
      });
    } catch (error) {
      // 6. Fail gracefully without leaking system internals
      return new Response("Internal Server Error", { status: 500 });
    }
  },
};
