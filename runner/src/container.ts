import { Container } from "@cloudflare/containers";

export class SalamSandbox extends Container {
  override defaultPort = 8080;
  override sleepAfter = "2m";
  override enableInternet = false;
}
