#include <stddef.h>

__attribute__((weak)) int SecTrustCopyAnchorCertificates(void **anchors) {
    if (anchors != NULL) {
        *anchors = NULL;
    }
    return -4;
}
