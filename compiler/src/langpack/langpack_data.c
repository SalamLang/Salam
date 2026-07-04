/*
 * Salam Programming Language (2024–2026)
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

#include "core/prelude.h"
#include "langpack/langpack.h"
#include "langpack/langpack_internal.h"

const kw_entry_t k_lang_en[] = {
    { TK_KW_FUNC,     "func"     },
    { TK_KW_RET,      "ret"      },
    { TK_KW_IF,       "if"       },
    { TK_KW_ELSE,     "else"     },
    { TK_KW_WHILE,    "while"    },
    { TK_KW_FOR,      "for"      },
    { TK_KW_MUT,      "mut"      },
    { TK_KW_CONST,    "const"    },
    { TK_KW_TYPE,     "type"     },
    { TK_KW_STRUCT,   "struct"   },
    { TK_KW_ENUM,     "enum"     },
    { TK_KW_END,      "end"      },
    { TK_KW_IMPORT,   "import"   },
    { TK_KW_AS,       "as"       },
    { TK_KW_TRUE,     "true"     },
    { TK_KW_FALSE,    "false"    },
    { TK_KW_NULL,     "null"     },
    { TK_KW_THIS,     "this"     },
    { TK_KW_BREAK,    "break"    },
    { TK_KW_CONTINUE, "continue" },
    { TK_KW_LAYOUT,   "layout"   },
    { TK_KW_PACKAGE,  "package"  },
    { TK_KW_PRINT,      "print"      },
    { TK_KW_PRINT,      "_"          },   
    { TK_KW_PRINTLN,    "println"    },
    { TK_KW_PRINTLN,    "__"         },   
    { TK_KW_PRINTERR,   "printerr"   },
    { TK_KW_PRINTERRLN, "printerrln" },
    
    { TK_KW_INPUT,      "input"      },
    { TK_KW_DEFER,    "defer"    },
    { TK_KW_OPERATOR, "operator" },
    { TK_KW_EXTERN,   "extern"   },
    { TK_KW_INTERFACE,"interface" },
    { TK_KW_PUB,      "pub"      },
    { TK_KW_COMPONENT,"component" },
    { TK_KW_REPEAT,   "repeat"   },
    { TK_KW_IMPL,     "impl"     },
    { TK_KW_TO,       "to"       },
    { TK_KW_STEP,     "every"    },
    { TK_EOF,         NULL       },
};
static const kw_entry_t k_lang_fa[] = {
    { TK_KW_FUNC,     "تابع"      },
    { TK_KW_RET,      "بازگشت"    },
    { TK_KW_IF,       "اگر"       },
    { TK_KW_ELSE,     "وگرنه"     },
    { TK_KW_WHILE,    "تاوقتی"    },
    { TK_KW_FOR,      "برای"      },
    { TK_KW_MUT,      "متغیر"     },
    { TK_KW_CONST,    "ثابت"      },
    { TK_KW_TYPE,     "نوع"       },
    { TK_KW_STRUCT,   "ساختار"    },
    { TK_KW_ENUM,     "شمارش"     },
    { TK_KW_END,      "پایان"     },
    { TK_KW_IMPORT,   "واردکردن"    },
    { TK_KW_AS,       "بعنوان"    },
    { TK_KW_TRUE,     "درست"      },
    { TK_KW_FALSE,    "نادرست"    },
    { TK_KW_NULL,     "پوچ"       },
    { TK_KW_THIS,     "این"       },
    { TK_KW_BREAK,    "بشکن"      },
    { TK_KW_CONTINUE, "ادامه"     },
    { TK_KW_LAYOUT,   "صفحه"      },
    { TK_KW_PACKAGE,  "بسته"      },
    { TK_KW_PRINT,      "بنویس"      },
    { TK_KW_PRINTLN,    "چاپ"        },
    { TK_KW_PRINTERR,   "خطابنویس"  },
    { TK_KW_PRINTERRLN, "خطاچاپ"    },
    { TK_KW_INPUT,      "بخوان"      },
    { TK_KW_DEFER,    "تعلیق"     },
    { TK_KW_OPERATOR, "عملگر"     },   
    { TK_KW_EXTERN,   "خارجی"     },   
    { TK_KW_INTERFACE,"واسط"      },   
    { TK_KW_PUB,      "عمومی"     },
    { TK_KW_COMPONENT,"مولفه"     },
    { TK_KW_REPEAT,   "تکرار"     },
    { TK_KW_IMPL,     "پیاده‌سازی" },
    { TK_KW_TO,       "تا"        },
    { TK_KW_STEP,     "هر"        },
    { TK_AND,         "و"         },
    { TK_OR,          "یا"        },
    { TK_EOF,         NULL        },
};
static const ctx_entry_t k_ctx_fa[] = {
    { "link",      "پیوند"   },
    { "static",    "ایستا"   },
    { "dynamic",   "پویا"    },
    { "framework", "چارچوب"  },
    { "kind",      "گونه"    },
    { NULL,        NULL      },
};
static const char k_entry_fa[] = "\xD8\xA7\xD8\xB5\xD9\x84\xDB\x8C";
struct langpack g_lang_en = { "en", k_lang_en, "main", NULL };
struct langpack g_lang_fa = { "fa", k_lang_fa, k_entry_fa, k_ctx_fa };
