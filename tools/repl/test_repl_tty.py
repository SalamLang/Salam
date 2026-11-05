import os
import pty
import select
import sys
import time

if len(sys.argv) < 2:
    print("usage: python3 tools/repl/test_repl_tty.py /path/to/salam [SALAM_STD]")
    sys.exit(2)

salam = sys.argv[1]
env = dict(os.environ, TERM="xterm")
if len(sys.argv) > 2:
    env["SALAM_STD"] = sys.argv[2]

pid, fd = pty.fork()
if pid == 0:
    os.execve(salam, [salam, "cli"], env)


def drain(seconds):
    out = b""
    end = time.time() + seconds
    while time.time() < end:
        ready, _, _ = select.select([fd], [], [], 0.2)
        if ready:
            try:
                out += os.read(fd, 4096)
            except OSError:
                break
    return out


failed = 0
banner = drain(3.0)
if b"salam>" not in banner:
    print("FAIL no prompt:", banner)
    failed += 1

for line, want in (("1 + 1", b"2"), ('println "hi"', b"hi")):
    os.write(fd, line.encode() + b"\r")
    out = drain(2.5)
    shown = out.decode(errors="replace").replace("\r", "\\r").replace("\n", "\\n")
    if b"^M" in out:
        print("FAIL Enter echoed ^M for", repr(line), shown)
        failed += 1
    elif want not in out:
        print("FAIL no result for", repr(line), shown)
        failed += 1
    else:
        print("ok  ", repr(line))

os.write(fd, b"\x04")
drain(1.0)
try:
    os.waitpid(pid, 0)
except OSError:
    pass
sys.exit(1 if failed else 0)
