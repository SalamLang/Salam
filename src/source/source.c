#include "core/prelude.h"
#include "source/source.h"

source_file_t *source_load(arena_t *a, const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
    long size = ftell(f);
    if (size < 0) { fclose(f); return NULL; }
    rewind(f);
    char *buf = (char *)arena_alloc(a, (size_t)size + 1);
    size_t got = fread(buf, 1, (size_t)size, f);
    fclose(f);
    buf[got] = '\0';
    source_file_t *sf = (source_file_t *)arena_alloc(a, sizeof(*sf));
    sf->path = arena_strdup(a, path);
    sf->text = buf;
    sf->len = got;
    return sf;
}
