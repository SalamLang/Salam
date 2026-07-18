// Runs a `salam js` bundle under Node with the DOM stub preloaded, then fires
// any scripted DOM events (see dom_stub.js) once the bundle's entry point has
// completed. Usage: node run_bundle.js <bundle.js>

const path = require("path");
const stub = require(path.join(__dirname, "dom_stub.js"));

const bundlePath = process.argv[2];
if (!bundlePath) {
  console.error("usage: node run_bundle.js <bundle.js>");
  process.exit(2);
}

require(path.resolve(bundlePath));
stub.fireAutoEvents();
