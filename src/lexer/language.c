#include "language.h"

language_map_t language_maps[] = {
    // First is default language
    {LANGUAGE_ENGLISH, en_keywords},
    {LANGUAGE_PERSIAN, fa_keywords},
    {-1, NULL},
};
