import { useEffect } from "react";

function App() {
  useEffect(() => {
    // 1. Define base URL from Vite
    const baseUrl = import.meta.env.BASE_URL;
    const version = "v=20260701";

    // 2. Define all the favicon and meta tags configuration
    const tagsConfig = [
      {
        tag: "link",
        attrs: {
          rel: "icon",
          type: "image/png",
          href: `${baseUrl}assets/icons/favicon-96x96.png?${version}`,
          sizes: "96x96",
        },
      },
      {
        tag: "link",
        attrs: {
          rel: "icon",
          type: "image/svg+xml",
          href: `${baseUrl}assets/icons/favicon.svg?${version}`,
        },
      },
      // Updated to point directly to the root fallback favicon
      {
        tag: "link",
        attrs: {
          rel: "shortcut icon",
          href: `${baseUrl}/favicon.ico?${version}`,
        },
      },
      {
        tag: "link",
        attrs: {
          rel: "apple-touch-icon",
          sizes: "180x180",
          href: `${baseUrl}assets/icons/apple-touch-icon.png?${version}`,
        },
      },
      {
        tag: "meta",
        attrs: { name: "apple-mobile-web-app-title", content: "Salam" },
      },
      {
        tag: "link",
        attrs: {
          rel: "manifest",
          href: `${baseUrl}assets/icons/site.webmanifest?${version}`,
        },
      },
    ];

    // 3. Create elements and append them to the <head>
    const createdElements: HTMLElement[] = [];

    tagsConfig.forEach(({ tag, attrs }) => {
      const el = document.createElement(tag);
      Object.entries(attrs).forEach(([key, value]) =>
        el.setAttribute(key, value),
      );
      document.head.appendChild(el);
      createdElements.push(el);
    });

    console.log("Bun + Vite skeleton loaded! Favicons injected dynamically.");

    // 4. Cleanup function to remove tags if the component unmounts
    return () => {
      createdElements.forEach((el) => el.remove());
    };
  }, []);

  return (
    <div className="min-h-screen bg-slate-900 text-white flex flex-col items-center justify-center p-4">
      <div className="max-w-xl text-center bg-slate-800 p-8 rounded-2xl shadow-xl border border-slate-700">
        <h1 className="text-4xl font-extrabold tracking-tight bg-gradient-to-r from-blue-400 to-emerald-400 bg-clip-text text-transparent">
          Bun + Vite + React + Tailwind v4
        </h1>
        <p className="mt-4 text-slate-400">
          Your static HTML and CSS migration skeleton is ready.
        </p>
        <button className="mt-6 px-6 py-2.5 bg-blue-600 hover:bg-blue-500 font-medium rounded-lg transition-colors shadow-lg shadow-blue-500/20">
          Get Started
        </button>
      </div>
    </div>
  );
}

export default App;
