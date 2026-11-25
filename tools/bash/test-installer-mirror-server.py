import functools
import http.server
import socket
import sys
import threading

root, portfile, logfile, mode = sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4]


class Handler(http.server.SimpleHTTPRequestHandler):
    def log_request(self, code="-", size="-"):
        with open(logfile, "a") as f:
            f.write("%s %s %s\n" % (self.command, self.path, code))

    def log_message(self, fmt, *args):
        pass


def serve_blackhole():
    s = socket.socket()
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind(("127.0.0.1", 0))
    s.listen(16)
    with open(portfile, "w") as f:
        f.write(str(s.getsockname()[1]))
    held = []
    while True:
        c, _ = s.accept()
        held.append(c)
        with open(logfile, "a") as f:
            f.write("CONNECT\n")


if mode == "blackhole":
    serve_blackhole()
else:
    handler = functools.partial(Handler, directory=root)
    srv = http.server.ThreadingHTTPServer(("127.0.0.1", 0), handler)
    with open(portfile, "w") as f:
        f.write(str(srv.server_address[1]))
    srv.serve_forever()
