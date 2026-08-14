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

/* The REPL's line editor: history plus in-place editing, hand-rolled per
 * platform so `salam cli` needs no readline/termcap dependency. Split out
 * of repl.c, which owns the session logic; the two share repl_internal.h.
 */

#include "driver/repl_internal.h"

#if defined(_WIN32)
#  include <process.h>
#  include <windows.h>
#  define REPL_USE_WINCON 1
#  define REPL_GETPID() _getpid()
#else
#  include <termios.h>
#  include <unistd.h>
#  define REPL_USE_WINCON 0
#  define REPL_GETPID() getpid()
#endif

#define REPL_HIST_MAX 200
#define REPL_LINE_MAX 4096
static char *repl_hist[REPL_HIST_MAX];
static int repl_nhist = 0;

void repl_hist_push(const char *line)
{
    if (!line || !line[0]) return;
    if (repl_nhist > 0 && strcmp(repl_hist[repl_nhist - 1], line) == 0) return;
    if (repl_nhist == REPL_HIST_MAX) {
        free(repl_hist[0]);
        memmove(repl_hist, repl_hist + 1, (REPL_HIST_MAX - 1) * sizeof *repl_hist);
        repl_nhist--;
    }
    repl_hist[repl_nhist++] = sal_strdup(line);
}

void repl_hist_clear(void)
{
    {
        int i = 0;
        for (; i < repl_nhist; i++)
            free(repl_hist[i]);
    }
    repl_nhist = 0;
}

#if REPL_USE_WINCON
/* Park the cursor `pos` characters into the line. */
static void repl_cursor(const char *prompt, const char *buf, int pos)
{
    printf("\r%s%.*s", prompt, pos, buf);
    fflush(stdout);
}

/* Redraw the line and park the cursor. `erase` is how many characters the
 * previous content occupied: a recalled history entry, a backspace or a
 * Ctrl-U usually leaves a shorter line than it replaced, and the old code
 * blanked a fixed width measured from column 0 - which covers the prompt
 * rather than the tail of the old text, so whatever hung past the new end
 * of line stayed on screen. */
static void repl_paint(const char *prompt, const char *buf, int len, int pos, int erase)
{
    int pad = (erase > len ? erase - len : 0) + 1;
    printf("\r%s%.*s%*s", prompt, len, buf, pad, "");
    repl_cursor(prompt, buf, pos);
}
#endif

char *repl_readline(const char *prompt)
{
#if REPL_USE_WINCON
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    if (hIn == INVALID_HANDLE_VALUE || GetFileType(hIn) != FILE_TYPE_CHAR) {
        printf("%s", prompt);
        fflush(stdout);
        char *buf = (char *)malloc(REPL_LINE_MAX);
        if (!buf || !fgets(buf, REPL_LINE_MAX, stdin)) {
            free(buf);
            return NULL;
        }
        size_t n = strlen(buf);
        while (n > 0 && (buf[n - 1] == '\n' || buf[n - 1] == '\r'))
            buf[--n] = '\0';
        return buf;
    }
    DWORD orig;
    GetConsoleMode(hIn, &orig);
    SetConsoleMode(hIn, ENABLE_PROCESSED_INPUT);
    printf("%s", prompt);
    fflush(stdout);
    char *buf = (char *)malloc(REPL_LINE_MAX);
    if (!buf) {
        SetConsoleMode(hIn, orig);
        return NULL;
    }
    int len = 0, pos = 0, hi = repl_nhist;
    char saved[REPL_LINE_MAX];
    saved[0] = '\0';
    int saved_len = 0;
    for (;;) {
        DWORD nr;
        INPUT_RECORD ev;
        if (!ReadConsoleInputA(hIn, &ev, 1, &nr)) break;
        if (ev.EventType != KEY_EVENT || !ev.Event.KeyEvent.bKeyDown) continue;
        WORD vk = ev.Event.KeyEvent.wVirtualKeyCode;
        char ch = ev.Event.KeyEvent.uChar.AsciiChar;
        if (vk == VK_RETURN) {
            putchar('\n');
            fflush(stdout);
            buf[len] = '\0';
            break;
        }
        if (vk == VK_BACK && pos > 0) {
            memmove(buf + pos - 1, buf + pos, (size_t)(len - pos));
            pos--;
            len--;
            repl_paint(prompt, buf, len, pos, len + 1);
            continue;
        }
        if (vk == VK_DELETE && pos < len) {
            memmove(buf + pos, buf + pos + 1, (size_t)(len - pos - 1));
            len--;
            repl_paint(prompt, buf, len, pos, len + 1);
            continue;
        }
        if (vk == VK_LEFT && pos > 0) {
            repl_cursor(prompt, buf, --pos);
            continue;
        }
        if (vk == VK_RIGHT && pos < len) {
            repl_cursor(prompt, buf, ++pos);
            continue;
        }
        if (vk == VK_HOME) {
            pos = 0;
            repl_cursor(prompt, buf, pos);
            continue;
        }
        if (vk == VK_END) {
            pos = len;
            repl_cursor(prompt, buf, pos);
            continue;
        }
        if (vk == VK_UP) {
            if (hi == repl_nhist) {
                memcpy(saved, buf, (size_t)len);
                saved[len] = '\0';
                saved_len = len;
            }
            if (hi > 0) {
                int was = len;
                int hlen = (int)strlen(repl_hist[hi - 1]);
                hi--;
                memcpy(buf, repl_hist[hi], (size_t)(hlen + 1));
                len = pos = hlen;
                repl_paint(prompt, buf, len, pos, was);
            }
            continue;
        }
        if (vk == VK_DOWN) {
            if (hi < repl_nhist) {
                int was = len;
                const char *h;
                int hlen;
                hi++;
                h = (hi == repl_nhist) ? saved : repl_hist[hi];
                hlen = (hi == repl_nhist) ? saved_len : (int)strlen(h);
                if (hlen > REPL_LINE_MAX - 1) hlen = REPL_LINE_MAX - 1;
                memcpy(buf, h, (size_t)hlen);
                buf[hlen] = '\0';
                len = pos = hlen;
                repl_paint(prompt, buf, len, pos, was);
            }
            continue;
        }
        if (ch == 1) {
            pos = 0;
            repl_cursor(prompt, buf, pos);
            continue;
        }
        if (ch == 5) {
            pos = len;
            repl_cursor(prompt, buf, pos);
            continue;
        }
        if (ch == 21) {
            int was = len;
            len = pos = 0;
            buf[0] = '\0';
            repl_paint(prompt, buf, len, pos, was);
            continue;
        }
        if (ch == 3 || (ch == 4 && len == 0)) {
            printf("\n");
            fflush(stdout);
            SetConsoleMode(hIn, orig);
            free(buf);
            return NULL;
        }
        if ((unsigned char)ch >= 32 && len < REPL_LINE_MAX - 1) {
            memmove(buf + pos + 1, buf + pos, (size_t)(len - pos));
            buf[pos] = ch;
            len++;
            pos++;
            repl_paint(prompt, buf, len, pos, len);
        }
    }
    SetConsoleMode(hIn, orig);
    return buf;
#else
    if (!isatty(STDIN_FILENO)) {
        fputs(prompt, stdout);
        fflush(stdout);
        char *buf = (char *)malloc(REPL_LINE_MAX);
        if (!buf || !fgets(buf, REPL_LINE_MAX, stdin)) {
            free(buf);
            return NULL;
        }
        size_t n = strlen(buf);
        while (n > 0 && (buf[n - 1] == '\n' || buf[n - 1] == '\r'))
            buf[--n] = '\0';
        return buf;
    }
    struct termios orig, raw;
    tcgetattr(STDIN_FILENO, &orig);
    raw = orig;
    raw.c_iflag &= ~((tcflag_t)(BRKINT | ICRNL | INPCK | ISTRIP | IXON));
    raw.c_cflag |= CS8;
    raw.c_lflag &= ~((tcflag_t)(ECHO | ICANON | IEXTEN | ISIG));
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    fputs(prompt, stdout);
    fflush(stdout);
    char *buf = (char *)malloc(REPL_LINE_MAX);
    if (!buf) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig);
        return NULL;
    }
    int len = 0, pos = 0, hi = repl_nhist;
    char saved[REPL_LINE_MAX];
    saved[0] = '\0';
    int saved_len = 0;
#  define WR(s, n)                                                                       \
      do {                                                                               \
          ssize_t _wr = write(STDOUT_FILENO, (s), (size_t)(n));                          \
          (void)_wr;                                                                     \
      } while (0)
#  define WRC(c)                                                                         \
      do {                                                                               \
          char _wc = (char)(c);                                                          \
          WR(&_wc, 1);                                                                   \
      } while (0)
    for (;;) {
        unsigned char c;
        if (read(STDIN_FILENO, &c, 1) <= 0) {
            if (len == 0) {
                free(buf);
                buf = NULL;
            }
            break;
        }
        if (c == '\r' || c == '\n') {
            WR("\r\n", 2);
            break;
        }
        if (c == 127 || c == 8) {
            if (pos > 0) {
                memmove(buf + pos - 1, buf + pos, (size_t)(len - pos));
                pos--;
                len--;
                WRC('\b');
                WR(buf + pos, len - pos);
                WRC(' ');
                {
                    int i = pos;
                    for (; i <= len; i++)
                        WRC('\b');
                }
                fflush(stdout);
            }
            continue;
        }
        if (c == 1) {
            {
                int i = 0;
                for (; i < pos; i++)
                    WRC('\b');
            }
            pos = 0;
            fflush(stdout);
            continue;
        }
        if (c == 5) {
            WR(buf + pos, len - pos);
            pos = len;
            fflush(stdout);
            continue;
        }
        if (c == 21) {
            {
                int i = 0;
                for (; i < pos; i++)
                    WRC('\b');
            }
            {
                int i = 0;
                for (; i < len; i++)
                    WRC(' ');
            }
            {
                int i = 0;
                for (; i < len; i++)
                    WRC('\b');
            }
            len = pos = 0;
            fflush(stdout);
            continue;
        }
        if (c == 3 || (c == 4 && len == 0)) {
            WR("\r\n", 2);
            fflush(stdout);
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig);
            free(buf);
            return NULL;
        }
        if (c == 27) {
            unsigned char s1, s2;
            if (read(STDIN_FILENO, &s1, 1) <= 0 || s1 != '[') continue;
            if (read(STDIN_FILENO, &s2, 1) <= 0) continue;
            if (s2 == 'A') {
                if (hi == repl_nhist) {
                    memcpy(saved, buf, (size_t)len);
                    saved[len] = '\0';
                    saved_len = len;
                }
                if (hi > 0) {
                    hi--;
                    {
                        int i = 0;
                        for (; i < pos; i++)
                            WRC('\b');
                    }
                    {
                        int i = 0;
                        for (; i < len; i++)
                            WRC(' ');
                    }
                    {
                        int i = 0;
                        for (; i < len; i++)
                            WRC('\b');
                    }
                    int hlen = (int)strlen(repl_hist[hi]);
                    memcpy(buf, repl_hist[hi], (size_t)(hlen + 1));
                    len = pos = hlen;
                    WR(buf, len);
                    fflush(stdout);
                }
            } else if (s2 == 'B') {
                if (hi < repl_nhist) {
                    hi++;
                    const char *h = (hi == repl_nhist) ? saved : repl_hist[hi];
                    int hlen = (hi == repl_nhist) ? saved_len : (int)strlen(h);
                    {
                        int i = 0;
                        for (; i < pos; i++)
                            WRC('\b');
                    }
                    {
                        int i = 0;
                        for (; i < len; i++)
                            WRC(' ');
                    }
                    {
                        int i = 0;
                        for (; i < len; i++)
                            WRC('\b');
                    }
                    memcpy(buf, h, (size_t)(hlen + 1));
                    len = pos = hlen;
                    WR(buf, len);
                    fflush(stdout);
                }
            } else if (s2 == 'C' && pos < len) {
                WR(buf + pos, 1);
                pos++;
                fflush(stdout);
            } else if (s2 == 'D' && pos > 0) {
                WRC('\b');
                pos--;
                fflush(stdout);
            } else if (s2 == '3') {
                unsigned char tilde;
                if (read(STDIN_FILENO, &tilde, 1) <= 0) continue;
                if (tilde == '~' && pos < len) {
                    memmove(buf + pos, buf + pos + 1, (size_t)(len - pos - 1));
                    len--;
                    WR(buf + pos, len - pos);
                    WRC(' ');
                    {
                        int i = pos;
                        for (; i <= len; i++)
                            WRC('\b');
                    }
                    fflush(stdout);
                }
            }
            continue;
        }
        if (c >= 32 && len < REPL_LINE_MAX - 1) {
            memmove(buf + pos + 1, buf + pos, (size_t)(len - pos));
            buf[pos] = (char)c;
            len++;
            WR(buf + pos, len - pos);
            {
                int i = pos + 1;
                for (; i < len; i++)
                    WRC('\b');
            }
            pos++;
            fflush(stdout);
        }
    }
#  undef WR
#  undef WRC
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig);
    if (buf) buf[len] = '\0';
    return buf;
#endif
}
