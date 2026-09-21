const path = require("node:path");
const stub = require(path.join(__dirname, "dom_stub.js"));

const bundlePath = process.argv[2];
if (!bundlePath) {
  console.error("usage: node run_bundle.js <bundle.js>");
  process.exit(2);
}

require(path.resolve(bundlePath));
stub.fireAutoEvents();
