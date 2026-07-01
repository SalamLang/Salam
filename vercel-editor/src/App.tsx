function App() {
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
