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
  },
});
