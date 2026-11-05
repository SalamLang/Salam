import { useCallback, useEffect, useRef, useState } from "react";
import { ApiError, transcribe, type Capabilities, type SpeechLang } from "./api";
import { IconClose, IconMic, IconStop } from "./icons";
import { isDesktop } from "./desktop";
import { isFa, t } from "./i18n";
import { toPersianDigits } from "./theme";
import { pushToast } from "./Toasts";

export type VoiceStage = "idle" | "asking" | "recording" | "sending";

const LANG_KEY = "salam-ai-speech-lang";
const AUDIO_TYPES = ["audio/webm;codecs=opus", "audio/ogg;codecs=opus", "audio/mp4;codecs=mp4a.40.2", "audio/mp4", "audio/webm"];
const BITS_PER_SECOND = 32000;
const MIN_SECONDS = 0.6;
const STOP_MARGIN_SECONDS = 1;
const BYTES_MARGIN = 65536;

function chooseType(): string | null {
  if (typeof MediaRecorder === "undefined" || !navigator.mediaDevices?.getUserMedia) return null;
  return AUDIO_TYPES.find((type) => MediaRecorder.isTypeSupported(type)) ?? "";
}

function savedLang(languages: SpeechLang[]): SpeechLang {
  try {
    const saved = localStorage.getItem(LANG_KEY);
    if (saved === "fa" || saved === "en") {
      if (languages.includes(saved)) return saved;
    }
  } catch {}
  const preferred: SpeechLang = isFa ? "fa" : "en";
  return languages.includes(preferred) ? preferred : (languages[0] ?? preferred);
}

export function clock(seconds: number): string {
  const whole = Math.max(0, Math.floor(seconds));
  const rest = whole % 60;
  return toPersianDigits(`${Math.floor(whole / 60)}:${rest < 10 ? "0" : ""}${rest}`);
}

function micProblem(error: unknown) {
  const name = error instanceof DOMException ? error.name : "";
  if (name === "NotAllowedError" || name === "SecurityError") {
    return {
      title: t("دسترسی به میکروفون داده نشد", "Microphone access was denied"),
      body: t("از تنظیمات مرورگر، اجازه‌ی میکروفون را برای این سایت فعال کنید.", "Allow the microphone for this site in your browser settings."),
    };
  }
  if (name === "NotFoundError" || name === "OverconstrainedError") {
    return {
      title: t("میکروفونی پیدا نشد", "No microphone found"),
      body: t("یک میکروفون وصل کنید و دوباره تلاش کنید.", "Connect a microphone and try again."),
    };
  }
  return {
    title: t("میکروفون در دسترس نیست", "The microphone is not available"),
    body: t("شاید برنامه‌ی دیگری از آن استفاده می‌کند.", "Another app may be using it."),
  };
}

export function useVoiceInput(caps: Capabilities | null, onSpoken: (text: string) => void) {
  const enabled = !isDesktop && caps?.speech === true && (caps.speech_languages?.length ?? 0) > 0;
  const languages = caps?.speech_languages ?? [];
  const limit = caps?.speech_max_seconds ?? 0;
  const maxBytes = caps?.speech_max_bytes ?? 0;
  const [stage, setStage] = useState<VoiceStage>("idle");
  const [seconds, setSeconds] = useState(0);
  const [language, setLanguage] = useState<SpeechLang | null>(null);
  const session = useRef(0);
  const stream = useRef<MediaStream | null>(null);
  const recorder = useRef<MediaRecorder | null>(null);
  const ticker = useRef(0);
  const aborter = useRef<AbortController | null>(null);
  const textSink = useRef(onSpoken);
  textSink.current = onSpoken;
  const chosen = language ?? savedLang(languages);
  const chosenRef = useRef(chosen);
  chosenRef.current = chosen;

  const release = useCallback(() => {
    window.clearInterval(ticker.current);
    for (const track of stream.current?.getTracks() ?? []) {
      track.onended = null;
      track.stop();
    }
    stream.current = null;
    recorder.current = null;
  }, []);

  const cancel = useCallback(() => {
    session.current += 1;
    const active = recorder.current;
    if (active && active.state !== "inactive") active.stop();
    aborter.current?.abort();
    aborter.current = null;
    release();
    setStage("idle");
    setSeconds(0);
  }, [release]);

  useEffect(() => cancel, [cancel]);

  const send = useCallback(async (audio: Blob, id: number) => {
    const control = new AbortController();
    aborter.current = control;
    setStage("sending");
    try {
      const result = await transcribe(audio, chosenRef.current, control.signal);
      if (id !== session.current) return;
      if (result.text.trim()) {
        textSink.current(result.text.trim());
      } else {
        pushToast({
          title: t("گفتاری شنیده نشد", "No speech was heard"),
          body: t("نزدیک‌تر به میکروفون صحبت کنید و دوباره تلاش کنید.", "Speak closer to the microphone and try again."),
        });
      }
    } catch (error) {
      if (id !== session.current || control.signal.aborted) return;
      const network = error instanceof ApiError && error.status === 0;
      pushToast({
        tone: network ? "network" : "failed",
        title: t("گفتار به متن تبدیل نشد", "Could not convert the speech to text"),
        body: error instanceof ApiError ? error.message : t("دوباره تلاش کنید.", "Please try again."),
      });
    } finally {
      if (id === session.current) {
        aborter.current = null;
        setStage("idle");
        setSeconds(0);
      }
    }
  }, []);

  const stop = useCallback(() => {
    const active = recorder.current;
    if (active && active.state !== "inactive") active.stop();
  }, []);

  const start = useCallback(async () => {
    if (!enabled) return;
    const type = chooseType();
    if (type === null) {
      pushToast({
        tone: "failed",
        title: t("این مرورگر نمی‌تواند صدا ضبط کند", "This browser cannot record audio"),
        body: t("با نسخه‌ی تازه‌ی Chrome، Firefox یا Safari امتحان کنید.", "Try a recent version of Chrome, Firefox or Safari."),
      });
      return;
    }
    const id = ++session.current;
    setStage("asking");
    let live: MediaStream;
    try {
      live = await navigator.mediaDevices.getUserMedia({
        audio: { channelCount: 1, echoCancellation: true, noiseSuppression: true, autoGainControl: true },
      });
    } catch (error) {
      if (id === session.current) {
        setStage("idle");
        pushToast({ tone: "failed", ...micProblem(error) });
      }
      return;
    }
    if (id !== session.current) {
      for (const track of live.getTracks()) track.stop();
      return;
    }
    let rec: MediaRecorder;
    try {
      rec = new MediaRecorder(live, { ...(type ? { mimeType: type } : {}), audioBitsPerSecond: BITS_PER_SECOND });
    } catch {
      for (const track of live.getTracks()) track.stop();
      setStage("idle");
      pushToast({ tone: "failed", title: t("این مرورگر نمی‌تواند صدا ضبط کند", "This browser cannot record audio") });
      return;
    }
    stream.current = live;
    recorder.current = rec;
    const chunks: Blob[] = [];
    let bytes = 0;
    let began = 0;
    let warned = false;
    const atLimit = () => {
      if (warned) return;
      warned = true;
      pushToast({
        title: t("به سقف زمان ضبط رسیدید", "You reached the recording limit"),
        body: t("همین مقدار به متن تبدیل می‌شود.", "What you recorded so far will be converted."),
      });
      stop();
    };
    rec.ondataavailable = (event) => {
      if (event.data.size === 0) return;
      chunks.push(event.data);
      bytes += event.data.size;
      if (maxBytes > 0 && bytes >= maxBytes - BYTES_MARGIN) atLimit();
    };
    rec.onerror = () => {
      if (id !== session.current) return;
      session.current += 1;
      release();
      setStage("idle");
      setSeconds(0);
      pushToast({ tone: "failed", title: t("ضبط صدا متوقف شد", "Recording stopped"), body: t("دوباره تلاش کنید.", "Please try again.") });
    };
    rec.onstop = () => {
      if (id !== session.current) return;
      const length = (performance.now() - began) / 1000;
      release();
      if (length < MIN_SECONDS || bytes === 0) {
        setStage("idle");
        setSeconds(0);
        pushToast({ title: t("ضبط خیلی کوتاه بود", "The recording was too short"), body: t("دکمه‌ی میکروفون را بزنید، صحبت کنید و دوباره آن را بزنید.", "Tap the microphone, speak, then tap it again.") });
        return;
      }
      void send(new Blob(chunks, { type: rec.mimeType || type || "audio/webm" }), id);
    };
    for (const track of live.getAudioTracks()) track.onended = stop;
    began = performance.now();
    rec.start(1000);
    setSeconds(0);
    setStage("recording");
    ticker.current = window.setInterval(() => {
      const passed = (performance.now() - began) / 1000;
      setSeconds(passed);
      if (limit > 0 && passed >= limit - STOP_MARGIN_SECONDS) atLimit();
    }, 250);
  }, [enabled, limit, maxBytes, release, send, stop]);

  const pickLanguage = useCallback((next: SpeechLang) => {
    setLanguage(next);
    try {
      localStorage.setItem(LANG_KEY, next);
    } catch {}
  }, []);

  return { enabled, stage, seconds, limit, languages, language: chosen, start, stop, cancel, pickLanguage };
}

export type VoiceInput = ReturnType<typeof useVoiceInput>;

export function VoiceButton({ voice, disabled }: { voice: VoiceInput; disabled?: boolean }) {
  if (!voice.enabled) return null;
  const recording = voice.stage === "recording";
  const waiting = voice.stage === "asking" || voice.stage === "sending";
  const label = recording ? t("پایان ضبط و تبدیل به متن", "Stop recording and convert to text") : t("ورودی صوتی", "Voice input");
  return (
    <button
      type="button"
      className={`attach-button voice-button ${recording ? "recording" : ""}`}
      onClick={recording ? voice.stop : voice.start}
      disabled={waiting || (disabled && !recording)}
      aria-label={label}
      aria-pressed={recording}
      title={label}
    >
      {recording ? <IconStop /> : <IconMic />}
    </button>
  );
}

export function VoiceBar({ voice }: { voice: VoiceInput }) {
  if (voice.stage === "idle") return null;
  const next: SpeechLang = voice.language === "fa" ? "en" : "fa";
  const canSwitch = voice.stage !== "sending" && voice.languages.length > 1;
  const message = voice.stage === "asking" ? t("منتظر اجازه‌ی میکروفون…", "Waiting for microphone permission…") : voice.stage === "sending" ? t("در حال تبدیل گفتار به متن…", "Converting speech to text…") : t("در حال ضبط", "Recording");
  return (
    <div className={`voice-bar ${voice.stage}`} role="status" aria-live="polite">
      {voice.stage === "recording" ? <span className="voice-dot" aria-hidden="true" /> : <span className="spinner tiny" aria-hidden="true" />}
      <span className="voice-message">{message}</span>
      {voice.stage === "recording" && (
        <span className="voice-time" dir="ltr" aria-label={t("زمان ضبط و سقف آن", "Recording time and its limit")}>
          {clock(voice.seconds)} / {clock(voice.limit)}
        </span>
      )}
      <span className="voice-actions">
        {canSwitch && (
          <button
            type="button"
            className="voice-lang"
            onClick={() => voice.pickLanguage(next)}
            aria-label={voice.language === "fa" ? t("زبان گفتار: فارسی. تغییر به انگلیسی", "Speech language: Persian. Switch to English") : t("زبان گفتار: انگلیسی. تغییر به فارسی", "Speech language: English. Switch to Persian")}
            title={t("زبانی که با آن صحبت می‌کنید", "The language you are speaking")}
          >
            {voice.language === "fa" ? t("فارسی", "Persian") : t("انگلیسی", "English")}
          </button>
        )}
        <button type="button" className="voice-cancel" onClick={voice.cancel} aria-label={t("لغو ضبط", "Cancel recording")} title={t("لغو", "Cancel")}>
          <IconClose width={14} height={14} />
          <span>{t("لغو", "Cancel")}</span>
        </button>
      </span>
    </div>
  );
}
