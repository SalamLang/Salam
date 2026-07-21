/*
 * Salam Programming Language (2024–2026)
 *
 *   +-------------------+
 *   |     S A L A M     |
 *   +-------------------+
 *
 * Designed by Seyyed Ali Mohammadiyeh and the Salam Team
 * Born from a decade of language design experience (since 2018)
 *
 * Repository: https://github.com/SalamLang/Salam
 *
 */

#include "jsgen/jsgen_internal.h"

static const js_host_t k_js_host[] = {
    {"salam_js_query", "document.querySelector(%0)"},
    {"salam_js_query_all", "Array.from(document.querySelectorAll(%0))"},
    {"salam_js_by_id", "document.getElementById(%0)"},
    {"salam_js_id_exists", "(document.getElementById(%0) !== null)"},
    {"salam_js_create", "document.createElement(%0)"},
    {"salam_js_body", "document.body"},
    {"salam_js_document", "document"},
    {"salam_js_window", "window"},
    {"salam_js_global", "globalThis[%0]"},
    {"salam_js_append", "%0.appendChild(%1)"},
    {"salam_js_remove", "%0.remove()"},
    {"salam_js_set_text", "(%0.textContent = %1)"},
    {"salam_js_get_text", "String(%0.textContent || \"\")"},
    {"salam_js_set_html", "(%0.innerHTML = %1)"},
    {"salam_js_set_value", "(%0.value = %1)"},
    {"salam_js_get_value", "String(%0.value || \"\")"},
    {"salam_js_set_style", "%0.style.setProperty(%1, %2)"},
    {"salam_js_get_style", "String(getComputedStyle(%0).getPropertyValue(%1))"},
    {"salam_js_set_attr", "%0.setAttribute(%1, %2)"},
    {"salam_js_get_attr", "String(%0.getAttribute(%1) || \"\")"},
    {"salam_js_add_class", "%0.classList.add(%1)"},
    {"salam_js_remove_class", "%0.classList.remove(%1)"},
    {"salam_js_toggle_class", "%0.classList.toggle(%1)"},
    {"salam_js_on", "%0.addEventListener(%1, %2)"},
    {"salam_js_on_e", "%0.addEventListener(%1, %2)"},
    {"salam_js_on_opts", "%0.addEventListener(%1, %2, %3)"},
    {"salam_js_off", "%0.removeEventListener(%1, %2)"},
    {"salam_js_on_window", "window.addEventListener(%0, %1)"},
    {"salam_js_on_window_e", "window.addEventListener(%0, %1)"},
    {"salam_js_on_ready", "document.addEventListener(\"DOMContentLoaded\", %0)"},
    {"salam_js_on_resize", "window.addEventListener(\"resize\", %0)"},
    {"salam_js_raf", "requestAnimationFrame(%0)"},
    {"salam_js_set_timeout", "setTimeout(%0, %1)"},
    {"salam_js_set_interval", "setInterval(%0, %1)"},
    {"salam_js_clear_timeout", "clearTimeout(%0)"},
    {"salam_js_clear_interval", "clearInterval(%0)"},
    {"salam_js_win_width", "window.innerWidth"},
    {"salam_js_win_height", "window.innerHeight"},
    {"salam_js_doc_width", "document.documentElement.clientWidth"},
    {"salam_js_doc_height", "document.documentElement.clientHeight"},
    {"salam_js_scroll_x", "window.scrollX"},
    {"salam_js_scroll_y", "window.scrollY"},
    {"salam_js_now", "performance.now()"},
    {"salam_js_get_prop", "%0[%1]"},
    {"salam_js_get_num", "Number(%0[%1] || 0)"},
    {"salam_js_get_str", "String(%0[%1] || \"\")"},
    {"salam_js_get_bool", "!!(%0[%1])"},
    {"salam_js_set_prop", "(%0[%1] = %2)"},
    {"salam_js_call", "%0[%1](%+2)"},
    {"salam_js_get_context", "%0.getContext(%1)"},
    {"salam_js_item_at", "%0[%1]"},
    {"salam_js_count_of", "Number(%0.length || 0)"},
    {"salam_js_event_target", "%0.target"},
    {"salam_js_event_x", "Number(%0.clientX || 0)"},
    {"salam_js_event_y", "Number(%0.clientY || 0)"},
    {"salam_js_event_key", "String(%0.key || \"\")"},
    {"salam_js_prevent_default", "%0.preventDefault()"},
    {"salam_js_stop_propagation", "%0.stopPropagation()"},
    {"salam_js_alert", "window.alert(%0)"},
    {"salam_js_confirm", "window.confirm(%0)"},
    {"salam_js_prompt", "String(window.prompt(%0) || \"\")"},
    {"salam_js_set_title", "(document.title = %0)"},
    {"salam_js_every_frame", "__sal_loop(%0)"},
    {"salam_js_random", "Math.random()"},
    {"salam_js_tone", "__sal_tone(%0, %1)"},
    {"salam_js_load_image", "__sal_img(%0)"},
    {"salam_js_img_ready", "!!(%0.complete && %0.naturalWidth > 0)"},
    {"salam_js_lsget_i", "__sal_lsget_i(%0)"},
    {"salam_js_lsset_i", "__sal_lsset(%0, %1)"},
    {"salam_js_lsget_s", "__sal_lsget_s(%0)"},
    {"salam_js_lsset_s", "__sal_lsset(%0, %1)"},
    {"salam_js_touch_x", "__sal_touch(%0, 0)"},
    {"salam_js_touch_y", "__sal_touch(%0, 1)"},
    {"salam_js_canvas_ctx", "document.getElementById(%0).getContext(\"2d\")"},
    {"salam_js_ctx_of", "%0.getContext(\"2d\")"},
    {"salam_js_canvas_w", "Number(%0.canvas.width)"},
    {"salam_js_canvas_h", "Number(%0.canvas.height)"},
    {"salam_js_fill_style", "(%0.fillStyle = %1)"},
    {"salam_js_stroke_style", "(%0.strokeStyle = %1)"},
    {"salam_js_line_width", "(%0.lineWidth = %1)"},
    {"salam_js_font", "(%0.font = %1)"},
    {"salam_js_text_align", "(%0.textAlign = %1)"},
    {"salam_js_global_alpha", "(%0.globalAlpha = %1)"},
    {"salam_js_fill_rect", "%0.fillRect(%1, %2, %3, %4)"},
    {"salam_js_stroke_rect", "%0.strokeRect(%1, %2, %3, %4)"},
    {"salam_js_clear_rect", "%0.clearRect(%1, %2, %3, %4)"},
    {"salam_js_round_rect", "__sal_rrect(%0, %1, %2, %3, %4, %5)"},
    {"salam_js_begin_path", "%0.beginPath()"},
    {"salam_js_close_path", "%0.closePath()"},
    {"salam_js_move_to", "%0.moveTo(%1, %2)"},
    {"salam_js_line_to", "%0.lineTo(%1, %2)"},
    {"salam_js_arc", "%0.arc(%1, %2, %3, %4, %5)"},
    {"salam_js_path_fill", "%0.fill()"},
    {"salam_js_path_stroke", "%0.stroke()"},
    {"salam_js_fill_text", "%0.fillText(%1, %2, %3)"},
    {"salam_js_ctx_save", "%0.save()"},
    {"salam_js_ctx_restore", "%0.restore()"},
    {"salam_js_translate", "%0.translate(%1, %2)"},
    {"salam_js_rotate", "%0.rotate(%1)"},
    {"salam_js_scale", "%0.scale(%1, %2)"},
    {"salam_js_draw_image", "%0.drawImage(%1, %2, %3, %4, %5)"},
};

const js_host_t *js_host_lookup(const char *name)
{
    {
        size_t i = 0;
        for (; i < sizeof(k_js_host) / sizeof(k_js_host[0]); i++)
            if (strcmp(k_js_host[i].name, name) == 0) return &k_js_host[i];
    }
    return NULL;
}
