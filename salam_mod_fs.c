#include "salam_mod_fs.h"

static int64_t _Salam_fs___strptr_str(const char* s);
static void* _Salam_fs___ld__push_void_ptr_i32_void_ptr_str(void* arr, int32_t n, void* cap__box, const char* name);
static bool _Salam_fs___ld__skip_str(const char* name);
static int64_t _Salam_fs___dname__off(void);

static int64_t _Salam_fs___strptr_str(const char* s) {
    void* v = ((void*)(intptr_t)(s));
    return ((int64_t)(intptr_t)(v));
}

salam_file* salam_open(const char* path, const char* mode) {
    void* fp = fopen(path, mode);
    if ((fp == NULL)) {
        return NULL;
    }
    return ((salam_file*)(fp));
}

const char* salam_file_read(salam_file* f, uint64_t size) {
    if ((f == NULL)) {
        return "";
    }
    void* fp = ((void*)(f));
    void* buf = _Salam_mem_AllocateZeroed_u64((size + ((uint64_t)(1))));
    fread(buf, ((uint64_t)(1)), size, fp);
    return ((const char*)(buf));
}

const char* salam_file_readline(salam_file* f) {
    if ((f == NULL)) {
        return "";
    }
    void* fp = ((void*)(f));
    const char* out = "";
    int32_t c = fgetc(fp);
    while ((c != (-1))) {
        if ((c == 10)) {
            return out;
        }
        out = salam_strcat(out, salam_char_from_code(c));
        c = fgetc(fp);
    }
    return out;
}

int32_t salam_file_write(salam_file* f, const char* data) {
    if ((f == NULL)) {
        return (0 - 1);
    }
    void* fp = ((void*)(f));
    const uint64_t n = ((uint64_t)((int32_t)strlen(data)));
    const uint64_t w = fwrite(((void*)(intptr_t)(data)), ((uint64_t)(1)), n, fp);
    return ((int32_t)(w));
}

void salam_file_close(salam_file* f) {
    if ((f == NULL)) {
        return;
    }
    fclose(((void*)(f)));
}

int32_t salam_file_seek(salam_file* f, int64_t off, int32_t whence) {
    if ((f == NULL)) {
        return (0 - 1);
    }
    void* fp = ((void*)(f));
    int32_t w = 0;
    if ((whence == 1)) {
        w = 1;
    }
    if ((whence == 2)) {
        w = 2;
    }
    if ((fseek(fp, ((int32_t)(off)), w) == 0)) {
        return 0;
    }
    return (0 - 1);
}

static void* _Salam_fs___ld__push_void_ptr_i32_void_ptr_str(void* arr, int32_t n, void* cap__box, const char* name) {
    int32_t* cap = ((int32_t*)(cap__box));
    void* a = arr;
    if ((n >= cap[0])) {
        if ((cap[0] == 0)) {
            cap[0] = 8;
        } else {
            cap[0] = (cap[0] * 2);
        }
        a = _Salam_mem_Reallocate_void_ptr_u64(a, ((uint64_t)((cap[0] * 8))));
    }
    int64_t* a64 = ((int64_t*)(a));
    a64[n] = _Salam_fs___strptr_str(salam_str_substr(name, 0, (int32_t)strlen(name)));
    return a;
}

static bool _Salam_fs___ld__skip_str(const char* name) {
    if ((strcmp(name, ".") == 0)) {
        return true;
    }
    if ((strcmp(name, "..") == 0)) {
        return true;
    }
    return false;
}

static int64_t _Salam_fs___dname__off(void) {
    return 19;
}

void* salam_os_listdir(const char* path, void* out__count) {
    int32_t* oc = ((int32_t*)(out__count));
    void* arr = NULL;
    int32_t n = 0;
    void* cap__box = _Salam_mem_AllocateZeroed_u64(((uint64_t)(4)));
    const int64_t off = _Salam_fs___dname__off();
    void* d = opendir(path);
    if ((d != NULL)) {
        void* e = readdir(d);
        while ((e != NULL)) {
            void* np = ((void*)(intptr_t)((((int64_t)(intptr_t)(e)) + off)));
            const char* nm = ((const char*)(np));
            if ((_Salam_fs___ld__skip_str(nm) == false)) {
                arr = _Salam_fs___ld__push_void_ptr_i32_void_ptr_str(arr, n, cap__box, nm);
                n = (n + 1);
            }
            e = readdir(d);
        }
        closedir(d);
    }
    _Salam_mem_Free_void_ptr(cap__box);
    if ((arr == NULL)) {
        arr = _Salam_mem_Allocate_u64(((uint64_t)(8)));
    }
    oc[0] = n;
    return arr;
}
