(function () {
"use strict";

let _d8_b9_d8_af_d8_af___d8_a7_d9_84_d9_86_d9_82_d8_b1_d8_a7_d8_aa = 0;

function _d8_b9_d8_b1_d8_b6___d8_a7_d9_84_d8_ad_d8_ac_d9_85() {
    const _d9_85_d8_b9_d9_84_d9_88_d9_85_d8_a7_d8_aa = document.querySelector("#info");
    _d9_85_d8_b9_d9_84_d9_88_d9_85_d8_a7_d8_aa.textContent = `النافذة: ${window.innerWidth} × ${window.innerHeight}`;
}

function _d8_a7_d9_84_d8_b1_d8_a6_db_8c_d8_b3_db_8c_d8_a9() {
    const _d8_b9_d9_86_d9_88_d8_a7_d9_86 = document.querySelector("#title");
    _d8_b9_d9_86_d9_88_d8_a7_d9_86.textContent = "سلام في المتصفح";
    _d8_b9_d9_86_d9_88_d8_a7_d9_86.style.setProperty("color", "#0a7f5f");
    const _d8_b2_d8_b1 = document.getElementById("btn");
    const _d8_ae_d8_b1_d8_ac = document.getElementById("out");
    const onClick = () => {
        _d8_b9_d8_af_d8_af___d8_a7_d9_84_d9_86_d9_82_d8_b1_d8_a7_d8_aa++;
        _d8_ae_d8_b1_d8_ac.textContent = `النقرات: ${_d8_b9_d8_af_d8_af___d8_a7_d9_84_d9_86_d9_82_d8_b1_d8_a7_d8_aa}`;
    };
    _d8_b2_d8_b1.addEventListener("click", onClick);
    window.addEventListener("resize", () => _d8_b9_d8_b1_d8_b6___d8_a7_d9_84_d8_ad_d8_ac_d9_85());
    _d8_b9_d8_b1_d8_b6___d8_a7_d9_84_d8_ad_d8_ac_d9_85();
    const _d8_b5_d9_86_d8_af_d9_88_d9_82 = document.createElement("div");
    _d8_b5_d9_86_d8_af_d9_88_d9_82.textContent = "أُنشئ بواسطة سلام";
    _d8_b5_d9_86_d8_af_d9_88_d9_82.classList.add("card");
    document.body.appendChild(_d8_b5_d9_86_d8_af_d9_88_d9_82);
}

// Run the program once the DOM is ready.
if (document.readyState === "loading") {
    document.addEventListener("DOMContentLoaded", _d8_a7_d9_84_d8_b1_d8_a6_db_8c_d8_b3_db_8c_d8_a9);
} else {
    _d8_a7_d9_84_d8_b1_d8_a6_db_8c_d8_b3_db_8c_d8_a9();
}
})();
