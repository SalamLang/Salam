import { fileURLToPath } from "node:url";
import { defineConfig } from "@rsbuild/core";
import { pluginReact } from "@rsbuild/plugin-react";

const grammar = fileURLToPath(new URL("../../extensions/vscode/syntaxes/salam.tmLanguage.json", import.meta.url));

export default defineConfig({
  plugins: [pluginReact()],
  source: {
    entry: { index: "./src/main.tsx" },
  },
  resolve: {
    alias: { "@salam/grammar": grammar },
  },
  html: {
    template: "./index.html",
    templateParameters: {
      apiUrl: (process.env.PUBLIC_API_URL || "https://ai-api.salamlang.ir").replace(/\/$/, ""),
    },
  },
  output: {
    distPath: { root: "dist" },
    cleanDistPath: true,
  },
  performance: {
    chunkSplit: { strategy: "split-by-experience" },
  },
  server: {
    port: 3000,
    headers: {
      "Accept-CH": "Sec-CH-UA-Platform-Version, Sec-CH-UA-Model, Sec-CH-UA-Full-Version-List, Sec-CH-UA-Arch, Sec-CH-UA-Bitness",
      "Permissions-Policy": "ch-ua-platform-version=(self), ch-ua-model=(self), ch-ua-full-version-list=(self), ch-ua-arch=(self), ch-ua-bitness=(self)",
    },
  },
});
