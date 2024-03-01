/*
 * Salam Programming Language (2024-2026)
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

/*
 * The libxml2 symbols LLVM references, so the toolchain links without
 * libxml2.
 *
 * The only libxml2 user in all of LLVM and LLD is libLLVMWindowsManifest.a,
 * which liblldCOFF.a pulls in to merge /manifestinput: files. Salam never
 * passes /manifestinput:, so lld never reaches the merger, but distro LLVM
 * builds (apt, Homebrew, MSYS2) are configured with LLVM_ENABLE_LIBXML2=ON
 * and the archive still names these sixteen symbols. Linking real libxml2
 * for them cost a static source build on every Linux job and a runtime
 * dependency that broke once already (Ubuntu 26.04 dropped libxml2.so.2).
 *
 * If something does reach the merger, xmlReadMemory reports a parse error
 * through the handler the merger installed, and merge() returns
 * "invalid xml document" instead of walking a null tree.
 *
 * An archive member is only pulled when something needs one of its symbols,
 * so with an LLVM built LLVM_ENABLE_LIBXML2=OFF this object is never linked.
 * Keep it dependency-free beyond libc.
 */

#include <stdlib.h>

typedef void (*salam_xml_error_func)(void *ctx, const char *msg, ...);

static void *salam_xml_error_ctx;
static salam_xml_error_func salam_xml_error_handler;

void (*xmlFree)(void *) = free;

void xmlSetGenericErrorFunc(void *ctx, salam_xml_error_func handler)
{
    salam_xml_error_ctx = ctx;
    salam_xml_error_handler = handler;
}

void *xmlReadMemory(const char *buffer, int size, const char *url, const char *encoding,
                    int options)
{
    (void)buffer;
    (void)size;
    (void)url;
    (void)encoding;
    (void)options;
    if (salam_xml_error_handler)
        salam_xml_error_handler(
            salam_xml_error_ctx, "%s",
            "Salam's LLVM is built without libxml2; manifest merging is unavailable\n");
    return NULL;
}

void *xmlNewDoc(const unsigned char *version)
{
    (void)version;
    return NULL;
}

void xmlFreeDoc(void *doc)
{
    (void)doc;
}

void *xmlDocGetRootElement(const void *doc)
{
    (void)doc;
    return NULL;
}

void *xmlDocSetRootElement(void *doc, void *root)
{
    (void)doc;
    (void)root;
    return NULL;
}

void xmlDocDumpFormatMemoryEnc(void *doc, unsigned char **mem, int *size,
                               const char *encoding, int format)
{
    (void)doc;
    (void)encoding;
    (void)format;
    if (mem) *mem = NULL;
    if (size) *size = 0;
}

void *xmlAddChild(void *parent, void *cur)
{
    (void)parent;
    (void)cur;
    return NULL;
}

void xmlUnlinkNode(void *cur)
{
    (void)cur;
}

void xmlFreeNode(void *cur)
{
    (void)cur;
}

void *xmlNewNs(void *node, const unsigned char *href, const unsigned char *prefix)
{
    (void)node;
    (void)href;
    (void)prefix;
    return NULL;
}

void *xmlCopyNamespace(void *cur)
{
    (void)cur;
    return NULL;
}

void xmlFreeNs(void *cur)
{
    (void)cur;
}

void *xmlNewProp(void *node, const unsigned char *name, const unsigned char *value)
{
    (void)node;
    (void)name;
    (void)value;
    return NULL;
}

unsigned char *xmlStrdup(const unsigned char *cur)
{
    (void)cur;
    return NULL;
}
