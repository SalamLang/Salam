// Cache for the Salam playground's compiler bundle.
//
// The bundle file names carry the Salam version, so a release never reads what
// an older release cached: a new version asks for file names that no cache has,
// and this worker then stores them under a new cache name. The old cache is
// dropped when this worker activates.
//
// Everything except those three files is left to the network, so the page and
// its assets always reflect the deployed release.

const SW_VERSION = "0.5.4-159c7d2463";
const CACHE_PREFIX = "salam-editor-";
const CACHE = `${CACHE_PREFIX}${SW_VERSION}`;

const ASSETS = [
  `./salam-wa-${SW_VERSION}.js`,
  `./salam-wa-${SW_VERSION}.wasm`,
  `./salam-wa-${SW_VERSION}.data`,
].map((asset) => new URL(asset, self.registration.scope).pathname);

self.addEventListener("install", (event) => {
  event.waitUntil(
    caches
      .open(CACHE)
      // One file at a time: a missing optional file must not leave the
      // worker uninstalled and the editor without a cache.
      .then((cache) =>
        Promise.allSettled(
          ASSETS.map((path) =>
            cache.add(new Request(path, { cache: "reload" })),
          ),
        ),
      )
      .then(() => self.skipWaiting()),
  );
});

self.addEventListener("activate", (event) => {
  event.waitUntil(
    caches
      .keys()
      .then((keys) =>
        Promise.all(
          keys
            .filter((key) => key.startsWith(CACHE_PREFIX) && key !== CACHE)
            .map((key) => caches.delete(key)),
        ),
      )
      .then(() => self.clients.claim()),
  );
});

self.addEventListener("fetch", (event) => {
  const request = event.request;
  if (request.method !== "GET") {
    return;
  }
  const url = new URL(request.url);
  if (url.origin !== self.location.origin || !ASSETS.includes(url.pathname)) {
    return;
  }
  const path = url.pathname;
  event.respondWith(
    caches.open(CACHE).then((cache) =>
      cache.match(path).then((hit) => {
        if (hit) {
          return hit;
        }
        return fetch(request).then((response) => {
          if (response && (response.ok || response.type === "opaque")) {
            cache.put(path, response.clone());
          }
          return response;
        });
      }),
    ),
  );
});
