import type { SalamSandbox } from "./container";

export interface RateLimiterBinding {
  limit(options: { key: string }): Promise<{ success: boolean }>;
}

export interface Env {
  SALAM_SANDBOX: DurableObjectNamespace<SalamSandbox>;
  RUNNER_RATE_LIMITER: RateLimiterBinding;
}
