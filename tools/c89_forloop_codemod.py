#!/usr/bin/env python3
# gnu89 for-loop codemod: rewrite C99 for-init declarations into a wrapping block so the loop counter keeps its for-scoped lifetime (no leak, no collisions).

import json, sys, subprocess, os

CLANG = os.environ.get("CLANG", "clang")
STD = os.environ.get("PARSE_STD", "gnu11")

def get_ast(path):
    cmd = [CLANG, "-Isrc", "-std=" + STD, "-ferror-limit=0",
           "-Xclang", "-ast-dump=json", "-fsyntax-only", path]
    r = subprocess.run(cmd, capture_output=True, text=True)
    if not r.stdout:
        sys.stderr.write("clang produced no AST for %s\n%s\n" % (path, r.stderr[:2000]))
        return None
    return json.loads(r.stdout)

def collect(path):
    """Return list of (for_begin, init_begin, init_end_excl_semi, init_endincl, for_end_after)
    edits for ForStmt-with-DeclStmt-init that live in `path`."""
    ast = get_ast(path)
    if ast is None:
        return None
    target = os.path.abspath(path)
    edits = []
    state = {"file": None, "macro": False}

    def visit_loc(loc):
        """Update current-file state from a loc/range-endpoint dict, clang writer order.
        Returns (offset_or_None, in_macro)."""
        if not isinstance(loc, dict):
            return None, False
        macro = ("spellingLoc" in loc) or ("expansionLoc" in loc)
        if "spellingLoc" in loc:
            visit_loc(loc["spellingLoc"])
        if "expansionLoc" in loc:
            visit_loc(loc["expansionLoc"])
        if "file" in loc:
            state["file"] = loc["file"]
        off = loc.get("offset")
        return off, macro

    def file_is_target():
        f = state["file"]
        if f is None:
            return False
        return os.path.abspath(f) == target

    def node_range(node):
        """Visit a node's loc+range in clang writer order; return dict of resolved
        offsets and flags captured AT THIS NODE (file state is global/stateful)."""
        info = {}
        if "loc" in node:
            visit_loc(node["loc"])
        rng = node.get("range", {})
        if "begin" in rng:
            info["begin_file"] = None
            boff, bmac = visit_loc(rng["begin"])
            info["begin_off"] = boff
            info["begin_macro"] = bmac
            info["begin_in_target"] = file_is_target()
        if "end" in rng:
            eoff, emac = visit_loc(rng["end"])
            info["end_off"] = eoff
            info["end_tok"] = rng["end"].get("tokLen", 0)
            info["end_macro"] = emac
            info["end_in_target"] = file_is_target()
        return info

    def walk(node):
        if not isinstance(node, dict):
            return
        info = node_range(node)
        if node.get("kind") == "ForStmt":
            inner = node.get("inner") or []
            if inner and isinstance(inner[0], dict) and inner[0].get("kind") == "DeclStmt":
                init = inner[0]
                for_begin = info.get("begin_off")
                for_end = info.get("end_off")
                for_end_tok = info.get("end_tok", 0)
                for_ok = (info.get("begin_in_target") and info.get("end_in_target")
                          and not info.get("begin_macro") and not info.get("end_macro")
                          and for_begin is not None and for_end is not None)
                iinfo = node_range(init)
                init_begin = iinfo.get("begin_off")
                init_end = iinfo.get("end_off")
                init_tok = iinfo.get("end_tok", 0)
                init_ok = (iinfo.get("begin_in_target") and iinfo.get("end_in_target")
                           and not iinfo.get("begin_macro") and not iinfo.get("end_macro")
                           and init_begin is not None and init_end is not None)
                if for_ok and init_ok:
                    edits.append({
                        "for_begin": for_begin,
                        "for_end_after": for_end + for_end_tok,
                        "init_begin": init_begin,
                        "init_semi": init_end,
                        "init_after_semi": init_end + init_tok,
                    })
                else:
                    edits.append({"skip": True, "for_begin": for_begin})
                for c in inner[1:]:
                    walk(c)
                return
        for c in (node.get("inner") or []):
            walk(c)

    walk(ast)
    return edits

def apply_edits(path):
    edits = collect(path)
    if edits is None:
        return None
    src = open(path, "rb").read()
    spans = []
    skipped = 0
    for e in edits:
        if e.get("skip"):
            skipped += 1
            continue
        decl_with_semi = src[e["init_begin"]:e["init_after_semi"]]
        spans.append((e["for_begin"], e["for_begin"], b"{ " + decl_with_semi + b" "))
        spans.append((e["init_begin"], e["init_semi"], b""))
        spans.append((e["for_end_after"], e["for_end_after"], b" }"))
    spans.sort(key=lambda s: (s[0], s[1]), reverse=True)
    out = src
    for start, end, rep in spans:
        out = out[:start] + rep + out[end:]
    return out, len([e for e in edits if not e.get("skip")]), skipped

def main():
    if len(sys.argv) < 2:
        sys.stderr.write("usage: c89_forloop_codemod.py [--apply] FILE...\n")
        sys.exit(2)
    args = sys.argv[1:]
    do_apply = False
    if args[0] == "--apply":
        do_apply = True
        args = args[1:]
    grand = 0
    for path in args:
        res = apply_edits(path)
        if res is None:
            print("ERROR %s (no AST)" % path)
            continue
        out, n, skipped = res
        grand += n
        tag = "APPLY" if do_apply else "DRY"
        extra = (" (%d skipped: header/macro)" % skipped) if skipped else ""
        print("%s %-44s %3d edits%s" % (tag, path, n, extra))
        if do_apply and n:
            open(path, "wb").write(out)
    print("TOTAL transformed for-loops: %d" % grand)

if __name__ == "__main__":
    main()
