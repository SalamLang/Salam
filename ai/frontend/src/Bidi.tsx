import { Fragment } from "react";

type Piece = { code: false; lines: string[] } | { code: true; text: string };

const FENCE = /^\s*(```|~~~)/;

function pieces(text: string): Piece[] {
  const out: Piece[] = [];
  let lines: string[] = [];
  let code: string[] | null = null;
  for (const line of text.split("\n")) {
    if (FENCE.test(line)) {
      if (code === null) {
        if (lines.length) out.push({ code: false, lines });
        lines = [];
        code = [];
      } else {
        out.push({ code: true, text: code.join("\n") });
        code = null;
      }
      continue;
    }
    if (code !== null) code.push(line);
    else lines.push(line);
  }
  if (code !== null) out.push({ code: true, text: code.join("\n") });
  if (lines.length) out.push({ code: false, lines });
  return out;
}

export function BidiText({ text, className }: { text: string; className?: string }) {
  return (
    <div className={`bidi-text ${className ?? ""}`}>
      {pieces(text).map((p, i) =>
        p.code ? (
          <pre key={i} dir="ltr" className="bidi-code">
            {p.text}
          </pre>
        ) : (
          <Fragment key={i}>
            {p.lines.map((line, j) =>
              line.trim() ? (
                <span key={j} dir="auto" className="bidi-line">
                  {line}
                </span>
              ) : (
                <span key={j} className="bidi-gap" aria-hidden="true" />
              ),
            )}
          </Fragment>
        ),
      )}
    </div>
  );
}
