import http from "node:http";

const zarinpal = new Map();
const zibal = new Map();
let track = 1000;

const read = (req) =>
  new Promise((resolve) => {
    let body = "";
    req.on("data", (d) => (body += d));
    req.on("end", () => {
      try {
        resolve(JSON.parse(body || "{}"));
      } catch {
        resolve({});
      }
    });
  });

http
  .createServer(async (req, res) => {
    const url = new URL(req.url, "http://x");
    const send = (code, obj) => {
      res.writeHead(code, { "Content-Type": "application/json" });
      res.end(JSON.stringify(obj));
    };
    if (req.method === "POST" && url.pathname === "/pg/v4/payment/request.json") {
      const b = await read(req);
      if (!b.merchant_id || !b.amount || !b.callback_url) return send(200, { data: [], errors: { code: -9, message: "اطلاعات ارسالی ناقص است" } });
      if (b.amount >= 100000000000) return send(200, { data: [], errors: { code: -11, message: "مبلغ بیش از حد مجاز است" } });
      const authority = `A${String(++track).padStart(35, "0")}`;
      zarinpal.set(authority, { amount: b.amount, verified: false });
      return send(200, { data: { code: 100, message: "Success", authority, fee_type: "Merchant", fee: 0 }, errors: [] });
    }
    if (req.method === "POST" && url.pathname === "/pg/v4/payment/verify.json") {
      const b = await read(req);
      const found = zarinpal.get(b.authority);
      if (!found) return send(200, { data: [], errors: { code: -51, message: "پرداخت ناموفق" } });
      if (Number(b.amount) !== Number(found.amount)) return send(200, { data: [], errors: { code: -54, message: "مبلغ با پرداخت نمی‌خواند" } });
      const code = found.verified ? 101 : 100;
      found.verified = true;
      return send(200, { data: { code, message: "Verified", ref_id: 987654321, card_pan: "502229******5995", fee_type: "Merchant", fee: 0 }, errors: [] });
    }
    if (req.method === "POST" && url.pathname === "/v1/request") {
      const b = await read(req);
      if (!b.merchant || !b.amount || !b.callbackUrl) return send(200, { result: 102, message: "merchant یافت نشد" });
      const id = ++track;
      zibal.set(String(id), { amount: b.amount, verified: false });
      return send(200, { result: 100, trackId: id, message: "success" });
    }
    if (req.method === "POST" && url.pathname === "/v1/verify") {
      const b = await read(req);
      const found = zibal.get(String(b.trackId));
      if (!found) return send(200, { result: 202, message: "قبلا تایید نشده" });
      const result = found.verified ? 201 : 100;
      found.verified = true;
      return send(200, { result, status: 1, amount: found.amount, refNumber: 55443322, cardNumber: "621986******1234", paidAt: new Date().toISOString(), message: "success" });
    }
    if (url.pathname.startsWith("/pg/StartPay/") || url.pathname.startsWith("/start/")) {
      res.writeHead(200, { "Content-Type": "text/html; charset=utf-8" });
      return res.end("<h1>gateway</h1>");
    }
    send(404, { message: "not found" });
  })
  .listen(Number(process.env.PORT ?? 18098), "127.0.0.1");
