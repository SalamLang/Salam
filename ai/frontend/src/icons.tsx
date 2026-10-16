import type { SVGProps } from "react";

type P = SVGProps<SVGSVGElement>;

function Base({ children, ...rest }: P) {
  return (
    <svg
      width="20"
      height="20"
      viewBox="0 0 24 24"
      fill="none"
      stroke="currentColor"
      strokeWidth="1.8"
      strokeLinecap="round"
      strokeLinejoin="round"
      aria-hidden="true"
      {...rest}
    >
      {children}
    </svg>
  );
}

export const IconPlus = (p: P) => (
  <Base {...p}>
    <path d="M12 5v14M5 12h14" />
  </Base>
);
export const IconSend = (p: P) => (
  <Base {...p}>
    <path d="M12 19V5M5 12l7-7 7 7" />
  </Base>
);
export const IconStop = (p: P) => (
  <Base {...p}>
    <rect x="7" y="7" width="10" height="10" rx="2" fill="currentColor" stroke="none" />
  </Base>
);
export const IconSun = (p: P) => (
  <Base {...p}>
    <circle cx="12" cy="12" r="4" />
    <path d="M12 2v2M12 20v2M4.9 4.9l1.4 1.4M17.7 17.7l1.4 1.4M2 12h2M20 12h2M4.9 19.1l1.4-1.4M17.7 6.3l1.4-1.4" />
  </Base>
);
export const IconMoon = (p: P) => (
  <Base {...p}>
    <path d="M20 14.5A8 8 0 0 1 9.5 4a8 8 0 1 0 10.5 10.5z" />
  </Base>
);
export const IconMenu = (p: P) => (
  <Base {...p}>
    <path d="M4 7h16M4 12h16M4 17h16" />
  </Base>
);
export const IconClose = (p: P) => (
  <Base {...p}>
    <path d="M6 6l12 12M18 6L6 18" />
  </Base>
);
export const IconTrash = (p: P) => (
  <Base {...p}>
    <path d="M4 7h16M10 11v6M14 11v6M6 7l1 12a2 2 0 0 0 2 2h6a2 2 0 0 0 2-2l1-12M9 7V4h6v3" />
  </Base>
);
export const IconEdit = (p: P) => (
  <Base {...p}>
    <path d="M4 20h4L19 9l-4-4L4 16v4zM14 6l4 4" />
  </Base>
);
export const IconCopy = (p: P) => (
  <Base {...p}>
    <rect x="9" y="9" width="11" height="11" rx="2" />
    <path d="M5 15V5a2 2 0 0 1 2-2h8" />
  </Base>
);
export const IconCheck = (p: P) => (
  <Base {...p}>
    <path d="M5 12l5 5L20 7" />
  </Base>
);
export const IconLogout = (p: P) => (
  <Base {...p}>
    <path d="M15 4h3a2 2 0 0 1 2 2v12a2 2 0 0 1-2 2h-3M10 17l-5-5 5-5M5 12h11" />
  </Base>
);
export const IconChat = (p: P) => (
  <Base {...p}>
    <path d="M21 12a8 8 0 0 1-11.6 7.1L4 20l1-4.6A8 8 0 1 1 21 12z" />
  </Base>
);
export const IconBook = (p: P) => (
  <Base {...p}>
    <path d="M4 5a2 2 0 0 1 2-2h14v16H6a2 2 0 0 0-2 2V5zM4 19a2 2 0 0 1 2-2h14" />
  </Base>
);
export const IconBug = (p: P) => (
  <Base {...p}>
    <rect x="7" y="8" width="10" height="12" rx="5" />
    <path d="M12 8V5M9 5l1.5 1.5M15 5l-1.5 1.5M3 13h4M17 13h4M4 19l3-2M20 19l-3-2M4 7l3 2M20 7l-3 2" />
  </Base>
);
export const IconCode = (p: P) => (
  <Base {...p}>
    <path d="M8 8l-5 4 5 4M16 8l5 4-5 4M14 4l-4 16" />
  </Base>
);
export const IconSwap = (p: P) => (
  <Base {...p}>
    <path d="M7 4L3 8l4 4M3 8h14M17 20l4-4-4-4M21 16H7" />
  </Base>
);
export const IconEye = (p: P) => (
  <Base {...p}>
    <path d="M2 12s3.5-7 10-7 10 7 10 7-3.5 7-10 7S2 12 2 12z" />
    <circle cx="12" cy="12" r="3" />
  </Base>
);
export const IconEyeOff = (p: P) => (
  <Base {...p}>
    <path d="M3 3l18 18M10.6 5.1A10.4 10.4 0 0 1 12 5c6.5 0 10 7 10 7a17 17 0 0 1-3.2 4.1M6.6 6.6A17 17 0 0 0 2 12s3.5 7 10 7a9.6 9.6 0 0 0 5.4-1.6M9.9 9.9a3 3 0 0 0 4.2 4.2" />
  </Base>
);
export const IconGlobe = (p: P) => (
  <Base {...p}>
    <circle cx="12" cy="12" r="9" />
    <path d="M3 12h18M12 3a14 14 0 0 1 0 18M12 3a14 14 0 0 0 0 18" />
  </Base>
);
export const IconPlug = (p: P) => (
  <Base {...p}>
    <path d="M9 3v4M15 3v4M6 7h12v4a6 6 0 0 1-12 0V7zM12 17v4" />
  </Base>
);
export const IconMonitor = (p: P) => (
  <Base {...p}>
    <rect x="3" y="4" width="18" height="12" rx="2" />
    <path d="M8 20h8M12 16v4" />
  </Base>
);
export const IconShield = (p: P) => (
  <Base {...p}>
    <path d="M12 3l8 3v6c0 4.5-3.4 8.2-8 9-4.6-.8-8-4.5-8-9V6l8-3z" />
  </Base>
);
export const IconBadgeCheck = (p: P) => (
  <Base {...p}>
    <path d="M12 3l8 3v6c0 4.5-3.4 8.2-8 9-4.6-.8-8-4.5-8-9V6l8-3z" />
    <path d="M8.5 12l2.5 2.5 4.5-5" />
  </Base>
);
export const IconAlert = (p: P) => (
  <Base {...p}>
    <path d="M12 4L2.5 20h19L12 4z" />
    <path d="M12 10v4M12 17v.5" />
  </Base>
);
export const IconChevronDown = (p: P) => (
  <Base {...p}>
    <path d="M6 9l6 6 6-6" />
  </Base>
);
export const IconActivity = (p: P) => (
  <Base {...p}>
    <path d="M3 12h4l3-7 4 14 3-7h4" />
  </Base>
);
export const IconSearch = (p: P) => (
  <Base {...p}>
    <circle cx="11" cy="11" r="6.5" />
    <path d="M16 16l4.5 4.5" />
  </Base>
);
export const IconLink = (p: P) => (
  <Base {...p}>
    <path d="M10 14a4 4 0 0 0 5.7 0l3-3a4 4 0 0 0-5.7-5.7l-1 1M14 10a4 4 0 0 0-5.7 0l-3 3a4 4 0 0 0 5.7 5.7l1-1" />
  </Base>
);
export const IconKey = (p: P) => (
  <Base {...p}>
    <circle cx="8" cy="15" r="4" />
    <path d="M11 12l9-9M16 7l3 3M14 9l2 2" />
  </Base>
);
export const IconFolder = (p: P) => (
  <Base {...p}>
    <path d="M3.5 7.5A1.5 1.5 0 0 1 5 6h4l2 2h8a1.5 1.5 0 0 1 1.5 1.5v8A1.5 1.5 0 0 1 19 19H5a1.5 1.5 0 0 1-1.5-1.5v-10z" />
  </Base>
);
export const IconTerminal = (p: P) => (
  <Base {...p}>
    <rect x="3" y="4.5" width="18" height="15" rx="2" />
    <path d="M7 9.5l3 2.5-3 2.5M12.5 15H17" />
  </Base>
);
export const IconFile = (p: P) => (
  <Base {...p}>
    <path d="M14 3.5H7A1.5 1.5 0 0 0 5.5 5v14A1.5 1.5 0 0 0 7 20.5h10a1.5 1.5 0 0 0 1.5-1.5V8L14 3.5z" />
    <path d="M14 3.5V8h4.5" />
  </Base>
);
export const IconSpark = (p: P) => (
  <Base {...p}>
    <path d="M12 3l1.8 4.9L19 9.7l-5.2 1.8L12 16.4l-1.8-4.9L5 9.7l5.2-1.8L12 3zM19 15l.8 2.2L22 18l-2.2.8L19 21l-.8-2.2L16 18l2.2-.8L19 15z" />
  </Base>
);
export const IconPackage = (p: P) => (
  <Base {...p}>
    <path d="M21 8l-9-5-9 5v8l9 5 9-5V8zM3 8l9 5 9-5M12 13v8" />
  </Base>
);
export const IconPlay = (p: P) => (
  <Base {...p}>
    <path d="M7 4.5v15l12-7.5-12-7.5z" />
  </Base>
);
export const IconTag = (p: P) => (
  <Base {...p}>
    <path d="M3 12V4h8l10 10-8 8L3 12z" />
    <circle cx="7.5" cy="8.5" r="1.3" />
  </Base>
);
