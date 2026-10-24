"""Checks a Mach-O executable written by the built-in linker.

sig <exe>              verify the ad-hoc code signature page hashes
cmp <ours> <ref>       compare disassembly with an ld64.lld reference after
                       taking out addresses, page numbers, local label
                       names and lld's GOT relaxations
"""
import difflib
import hashlib
import re
import struct
import subprocess
import sys


def check_sig(path):
    d = open(path, 'rb').read()
    ncmds = struct.unpack_from('<I', d, 16)[0]
    off = 32
    cs = None
    for _ in range(ncmds):
        cmd, size = struct.unpack_from('<II', d, off)
        if cmd == 0x1d:
            cs = struct.unpack_from('<II', d, off + 8)
        off += size
    if cs is None:
        return 'no LC_CODE_SIGNATURE'
    do, _ = cs
    if struct.unpack_from('>I', d, do)[0] != 0xfade0cc0:
        return 'bad SuperBlob magic'
    cd = do + struct.unpack_from('>I', d, do + 16)[0]
    f = struct.unpack_from('>IIIIIIIII', d, cd)
    if f[0] != 0xfade0c02:
        return 'bad CodeDirectory magic'
    hoff, ncode, limit, shift = f[4], f[7], f[8], d[cd + 39]
    for i in range(ncode):
        blk = d[i << shift:min((i + 1) << shift, limit)]
        want = d[cd + hoff + 32 * i:cd + hoff + 32 * (i + 1)]
        if hashlib.sha256(blk).digest() != want:
            return 'page %d hash mismatch' % i
    return ''


def norm(path):
    out = subprocess.run(['llvm-objdump', '--macho', '-d', '--no-show-raw-insn', '--no-leading-addr', path],
                         capture_output=True, text=True).stdout
    lines = []
    for ln in out.split('\n'):
        ln = re.sub(r'0x[0-9a-f]+', '', ln)
        ln = re.sub(r'#-?[0-9]+', '', ln)
        ln = re.sub(r'adrp\t(x\d+), -?\d+', r'adrp \1', ln)
        ln = ln.replace('__mh_execute_header', '')
        ln = re.sub(r'\b[lL]_[\w.$]+(?=\(%rip\))', '', ln)
        ln = re.sub(r', \[(x\d+), #\]', r', [\1]', ln)
        if not ln or ln.startswith('/') or re.match(r'^(l_|ltmp)', ln) or ln.endswith(':') and ln.startswith(path):
            continue
        lines.append(ln)
    return lines


BENIGN = [r'adrp', r'ldr\tx\d+, \[x\d+\]$', r'add\tx\d+, x\d+, #$', r'movq\t\(%rip\), %r',
          r'leaq\t\(%rip\), %r', r'leaq\tl_\w+\(%rip\)', r'leaq\t-\(%rip\)', r':$']


def compare(ours, ref):
    bad = []
    for ln in difflib.unified_diff(norm(ours), norm(ref), lineterm='', n=0):
        if ln[:1] in '+-' and not ln.startswith(('+++', '---')):
            if not any(re.search(b, ln) for b in BENIGN):
                bad.append(ln)
    return bad


def main():
    if len(sys.argv) >= 3 and sys.argv[1] == 'sig':
        err = check_sig(sys.argv[2])
        if err:
            print(err)
            return 1
        return 0
    if len(sys.argv) >= 4 and sys.argv[1] == 'cmp':
        bad = compare(sys.argv[2], sys.argv[3])
        for ln in bad[:5]:
            print(ln)
        return 1 if bad else 0
    print(__doc__)
    return 2


if __name__ == '__main__':
    sys.exit(main())
