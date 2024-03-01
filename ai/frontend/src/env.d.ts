interface ImportMetaEnv {
  readonly PUBLIC_API_URL?: string;
}

interface ImportMeta {
  readonly env: ImportMetaEnv;
}

declare module "@salam/grammar" {
  const grammar: Record<string, unknown>;
  export default grammar;
}
