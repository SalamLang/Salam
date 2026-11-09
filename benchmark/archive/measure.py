import os
import resource
import subprocess
import sys
import time


def main():
    out_path = sys.argv[1]
    cmd = sys.argv[2:]
    t0 = time.monotonic()
    p = subprocess.Popen(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    _, status, ru = os.wait4(p.pid, 0)
    wall = time.monotonic() - t0
    rc = os.waitstatus_to_exitcode(status)
    with open(out_path, "w") as f:
        f.write("%d %.3f %.3f %.3f %d\n" % (rc, wall, ru.ru_utime, ru.ru_stime, ru.ru_maxrss))


main()
