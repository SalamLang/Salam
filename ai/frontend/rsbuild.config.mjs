import { defineConfig } from "@rsbuild/core";
import { pluginReact } from "@rsbuild/plugin-react";

export default defineConfig({
  plugins: [pluginReact()],
  source: {
    entry: { index: "./src/main.tsx" },
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
