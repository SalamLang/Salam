<?php

// The php -S equivalent of main.salam. Same routes, same response bodies, so
// the only thing that differs between the two numbers is the server.
//
//   php -S 127.0.0.1:8101 -t tests/en/apps/httpbench/bench/php \
//       tests/en/apps/httpbench/bench/php/index.php
//
// php -S is a single-process, single-request-at-a-time development server.
// That is not a criticism of PHP, it is what the manual says the flag is for.
// It is here because "php -S" is what the request named, and because it marks
// the bottom of the range the other three sit in.

declare(strict_types=1);

$env    = getenv('HTTPBENCH_ASSETS');
$ASSETS = ($env === false || $env === '') ? __DIR__ . '/../../public' : $env;

// Read once at boot, which is the whole point of the /cached route. An
// unreadable asset leaves it empty rather than false, so the route answers
// instead of dying on json_ok()'s string parameter.
$cached = file_get_contents($ASSETS . '/data.json');
$CACHED = $cached === false ? '' : $cached;

$uri    = $_SERVER['REQUEST_URI'] ?? '/';
$method = $_SERVER['REQUEST_METHOD'] ?? 'GET';
$qpos   = strpos($uri, '?');
$path   = $qpos === false ? $uri : substr($uri, 0, $qpos);

function text(string $body): void
{
    header('Content-Type: text/plain; charset=utf-8');
    echo $body;
}

function json_ok(string $body): void
{
    header('Content-Type: application/json');
    echo $body;
}

function int_query(string $name, int $fallback, int $lo, int $hi): int
{
    // A query parameter can arrive as an array (?n[]=1), so anything that is
    // not a plain string falls back rather than being cast.
    $raw = $_GET[$name] ?? '';
    if (!is_string($raw) || $raw === '') {
        return $fallback;
    }
    $v = (int)$raw;
    return max($lo, min($hi, $v));
}

function row(string $p, string $note): string
{
    return "<tr><td><a href=\"$p\">$p</a></td><td>$note</td></tr>\n";
}

// One path parameter, matched the way a micro-framework would.
if (preg_match('#^/users/([^/]+)$#', $path, $m)) {
    $id = $m[1];
    json_ok("{\"id\":\"$id\",\"name\":\"user-$id\",\"active\":true}");
    return;
}

switch ($path) {
    case '/plaintext':
        text('Hello, World!');
        return;

    case '/health':
        json_ok('{"status":"ok","service":"httpbench"}');
        return;

    case '/json':
        json_ok('{"message":"Hello, World!","server":"php","routes":13,"ok":true}');
        return;

    case '/file':
        $body = file_get_contents($ASSETS . '/data.json');
        if ($body === false || $body === '') {
            http_response_code(500);
            json_ok('{"error":"asset missing"}');
            return;
        }
        json_ok($body);
        return;

    case '/cached':
        json_ok($CACHED);
        return;

    case '/search':
        $raw = $_GET['q'] ?? '';
        $q = is_string($raw) ? $raw : '';
        $n = int_query('n', 5, 0, 100);
        $out = '{"query":"' . $q . '","count":' . $n . ',"results":[';
        for ($i = 0; $i < $n; $i++) {
            if ($i > 0) {
                $out .= ',';
            }
            $r = $i + 1;
            $out .= '{"rank":' . $r . ',"title":"' . $q . ' result ' . $r . '"}';
        }
        json_ok($out . ']}');
        return;

    case '/compute':
        $n = int_query('n', 1000, 0, 5000000);
        $acc = 0;
        for ($i = 0; $i < $n; $i++) {
            $acc += ($i * $i) % 7;
        }
        json_ok('{"n":' . $n . ',"sum":' . $acc . '}');
        return;

    case '/headers':
        json_ok(json_encode([
            'host'       => $_SERVER['HTTP_HOST'] ?? '',
            'user_agent' => $_SERVER['HTTP_USER_AGENT'] ?? '',
            'accept'     => $_SERVER['HTTP_ACCEPT'] ?? '',
            'method'     => $method,
            'path'       => $path,
        ], JSON_UNESCAPED_SLASHES | JSON_THROW_ON_ERROR));
        return;

    case '/echo':
        $raw = file_get_contents('php://input');
        $body = $raw === false ? '' : $raw;
        json_ok(json_encode(
            ['bytes' => strlen($body), 'echo' => $body],
            JSON_UNESCAPED_SLASHES | JSON_THROW_ON_ERROR
        ));
        return;

    case '/':
        header('Content-Type: text/html; charset=utf-8');
        echo "<!doctype html>\n<html lang=\"en\">\n<head>\n";
        echo "<meta charset=\"utf-8\">\n";
        echo "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n";
        echo "<title>Salam httpbench</title>\n";
        echo "<link rel=\"stylesheet\" href=\"/static/style.css\">\n";
        echo "</head>\n<body>\n<main>\n";
        echo "<h1>Salam httpbench</h1>\n";
        echo "<p>A server built to be measured. Each route isolates one cost.</p>\n";
        echo "<table>\n<tr><th>Route</th><th>What it costs</th></tr>\n";
        echo row('/plaintext', 'the floor: accept, parse, route, write');
        echo row('/json', 'small-object serialization');
        echo row('/', 'this page, assembled per request');
        echo row('/file', 'a disk read on every request');
        echo row('/cached', 'the same bytes, read once at boot');
        echo row('/static/style.css', 'the built-in static file path');
        echo row('/users/:id', 'one router-extracted path parameter');
        echo row('/search?q=salam&n=5', 'query-string parsing');
        echo row('/compute?n=1000', 'tunable CPU work');
        echo row('/headers', 'request headers walked and echoed');
        echo row('/echo (POST)', 'request body read back out');
        echo row('/health', "a load balancer's poll");
        echo "</table>\n</main>\n</body>\n</html>\n";
        return;
}

// /static/... is served off disk. Under `php -S` a request that maps to a real
// file never reaches this script at all, so this branch only runs when the
// router is mounted behind something else.
if (str_starts_with($path, '/static/')) {
    $rel  = substr($path, strlen('/static/'));
    $real = realpath($ASSETS . '/' . $rel);
    $root = realpath($ASSETS);
    if ($real !== false && $root !== false && str_starts_with($real, $root) && is_file($real)) {
        header('Content-Type: ' . (str_ends_with($real, '.css') ? 'text/css' : 'application/octet-stream'));
        readfile($real);
        return;
    }
}

http_response_code(404);
text('404 Not Found');
