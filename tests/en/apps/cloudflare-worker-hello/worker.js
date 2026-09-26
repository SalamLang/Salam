import { createWorker } from "../../../../std/net/http/cloudflare/worker.js";
import app from "./build/app.wasm";

export default createWorker(app);
