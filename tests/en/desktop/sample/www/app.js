const out = document.getElementById("out");

document.getElementById("ask").addEventListener("click", async () => {
  out.textContent = await salam.invoke("greet", { name: "دنیا" });
});

salam.invoke("start", {}).then((auto) => {
  if (auto) {
    salam
      .invoke("greet", { name: "test" })
      .then((text) => salam.invoke("finish", { text }));
  }
});
