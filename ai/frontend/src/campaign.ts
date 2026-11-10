import { api } from "./api";

const VID_KEY = "salam-campaign-vid";
const FIELDS_KEY = "salam-campaign-fields";
const MAX_AGE_MS = 30 * 24 * 60 * 60 * 1000;

export type CampaignFields = {
  vid: string;
  source: string;
  medium: string;
  campaign: string;
  term: string;
  content: string;
};

const EMPTY: CampaignFields = {
  vid: "",
  source: "",
  medium: "",
  campaign: "",
  term: "",
  content: "",
};

function randomId(): string {
  try {
    return crypto.randomUUID().replace(/-/g, "");
  } catch {
    return Math.random().toString(36).slice(2) + Date.now().toString(36);
  }
}

function read(): { fields: CampaignFields; at: number } | null {
  try {
    const raw = localStorage.getItem(FIELDS_KEY);
    if (!raw) return null;
    const parsed = JSON.parse(raw) as { fields: CampaignFields; at: number };
    if (!parsed || typeof parsed.at !== "number") return null;
    return parsed;
  } catch {
    return null;
  }
}

function write(fields: CampaignFields) {
  try {
    localStorage.setItem(
      FIELDS_KEY,
      JSON.stringify({ fields, at: Date.now() }),
    );
  } catch {}
}

export function getCampaignFields(): CampaignFields {
  const found = read();
  if (!found) return EMPTY;
  if (Date.now() - found.at > MAX_AGE_MS) return EMPTY;
  return found.fields;
}

/**
 * Reads utm_source/utm_medium/utm_campaign/utm_term/utm_content from the
 * current URL, the standard UTM parameter family marketing tools and ad
 * platforms already generate links with. If a campaign is present, it is
 * kept in localStorage for 30 days so a later signup or payment in this
 * browser can still be attributed to it, and a visit is logged.
 */
export function captureFromUrl(): void {
  const params = new URLSearchParams(location.search);
  const source = params.get("utm_source") ?? "";
  const campaign = params.get("utm_campaign") ?? "";
  if (!source && !campaign) return;
  let vid = "";
  try {
    vid = localStorage.getItem(VID_KEY) ?? "";
  } catch {}
  if (!vid) {
    vid = randomId();
    try {
      localStorage.setItem(VID_KEY, vid);
    } catch {}
  }
  const fields: CampaignFields = {
    vid,
    source,
    medium: params.get("utm_medium") ?? "",
    campaign,
    term: params.get("utm_term") ?? "",
    content: params.get("utm_content") ?? "",
  };
  write(fields);
  api
    .recordVisit({
      ...fields,
      path: location.pathname,
      referrer: document.referrer.slice(0, 200),
    })
    .catch(() => {});
}
