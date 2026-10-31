#include "salam_engine.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int failures;

static void expect(int ok, const char *what) {
    printf("%s %s\n", ok ? "ok  " : "FAIL", what);
    fflush(stdout);
    if (!ok) {
        failures++;
    }
}

static void write_file(const char *path, const char *text) {
    FILE *f = fopen(path, "w");
    fputs(text, f);
    fclose(f);
}

static void *stopper(void *arg) {
    (void)arg;
    usleep(300000);
    salam_engine_stop();
    return NULL;
}

static int run(const char *cmd, const char *path) {
    char args[1024];
    snprintf(args, sizeof args, "%s\n%s", cmd, path);
    int rc = salam_engine_cli(args);
    fflush(stdout);
    return rc;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "usage: %s STD_DIR WORK_DIR\n", argv[0]);
        return 2;
    }
    const char *dir = argv[2];
    char ok_path[512], exit_path[512], loop_path[512], fa_path[512];
    snprintf(ok_path, sizeof ok_path, "%s/ok.salam", dir);
    snprintf(exit_path, sizeof exit_path, "%s/exit.salam", dir);
    snprintf(loop_path, sizeof loop_path, "%s/loop.salam", dir);
    snprintf(fa_path, sizeof fa_path, "%s/fa.salam", dir);
    write_file(ok_path, "func main:\n    n := input \"name? \"\n    println \"hi \" + n\n    println 7\nend\n");
    write_file(exit_path, "import os\nfunc leave(code: int):\n    os.Exit(code)\nend\nfunc main:\n    println \"bye\"\n    leave(3)\n    println \"never\"\nend\n");
    write_file(loop_path, "func main:\n    mut i := 0\n    until true:\n        i += 1\n    end\nend\n");
    write_file(fa_path, "تابع ریشه:\n    سرچاپ «سلام دنیا»\nپایان\n");

    salam_engine_boot(argv[1]);

    char *v = salam_engine_version();
    expect(v && strlen(v) > 0, "version");
    printf("     %s\n", v);
    salam_engine_free(v);

    const char *src = "func main:\n    // note\n    x := \"\xd8\xb3\xd9\x84\xd8\xa7\xd9\x85\"\n    println x, 42, len(x)\nend\n";
    char *t = salam_engine_tokens(src, "auto");
    printf("     tokens %s\n", t);
    expect(t && strstr(t, "1,0,4;") != NULL, "tokens: 'func' is a keyword at 0..4");
    expect(t && strstr(t, "4,15,7;") != NULL, "tokens: comment at utf16 15..22");
    expect(t && strstr(t, "2,32,6;") != NULL, "tokens: persian string counted in utf16 units");
    expect(t && strstr(t, "3,54,2;") != NULL, "tokens: number after the persian string");
    salam_engine_free(t);

    char *e = salam_engine_tokens("", "auto");
    expect(e && strlen(e) == 0, "tokens: empty source");
    salam_engine_free(e);

    char *c = salam_engine_check("func main:\n    x := 1\n    println y\nend\n", "/tmp/a.salam", "auto");
    printf("     check %s\n", c);
    expect(c && strstr(c, "\"E001\"") && strstr(c, "\"line\":3") && strstr(c, "\"length\":1") && !strstr(strstr(c, "E001") + 4, "E001"), "check: unknown identifier on line 3");
    salam_engine_free(c);

    char *c2 = salam_engine_check(src, "/tmp/a.salam", "auto");
    expect(c2 && strlen(c2) == 0, "check: clean source has no diagnostics");
    salam_engine_free(c2);

    char *c3 = salam_engine_check("func main:\n    x := (\nend\n", "/tmp/a.salam", "auto");
    expect(c3 && strstr(c3, "\"level\":\"error\"") != NULL, "check: parse error is reported as json");
    salam_engine_free(c3);

    char *d = salam_engine_detect("// lang: fa\n");
    expect(d && strcmp(d, "fa") == 0, "detect: persian source");
    salam_engine_free(d);

    expect(run("exec", ok_path) == 0, "exec: reads stdin and exits 0");
    expect(run("exec", exit_path) == 3, "exec: os.Exit(3) returns 3 and keeps the host alive");

    pthread_t th;
    salam_engine_clear_stop();
    pthread_create(&th, NULL, stopper, NULL);
    expect(run("exec", loop_path) == 130, "exec: stop ends an endless loop with 130");
    pthread_join(th, NULL);
    salam_engine_clear_stop();

    expect(run("exec", fa_path) == 0, "exec: persian program");
    expect(run("exec", exit_path) == 3, "exec: second run after a stop");
    expect(run("check", ok_path) == 0, "cli: check");

    printf("%d failure(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
