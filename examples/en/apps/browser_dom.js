(function () {
"use strict";

let clicks = 0;

function show_size() {
    const info = document.querySelector("#info");
    info.textContent = `window: ${window.innerWidth} x ${window.innerHeight}`;
}

function main() {
    const title = document.querySelector("#title");
    title.textContent = "Salam in the browser";
    title.style.setProperty("color", "#0a7f5f");
    const btn = document.getElementById("btn");
    const out = document.getElementById("out");
    const onClick = () => {
        clicks++;
        out.textContent = `clicks: ${clicks}`;
    };
    btn.addEventListener("click", onClick);
    window.addEventListener("resize", () => show_size());
    show_size();
    const box = document.createElement("div");
    box.textContent = "created from Salam";
    box.classList.add("card");
    document.body.appendChild(box);
}

// Run the program once the DOM is ready.
if (document.readyState === "loading") {
    document.addEventListener("DOMContentLoaded", main);
} else {
    main();
}
})();
