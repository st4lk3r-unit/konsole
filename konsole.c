#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "konsole.h"
#include "konsole_priv.h"

static inline void ts_puts(struct konsole *ks, const char *s) {
    if (ks->io.write) ks->io.write(ks->io.ctx, (const uint8_t*)s, strlen(s));
}
static inline void ts_write(struct konsole *ks, const uint8_t *b, size_t n) {
    if (ks->io.write) ks->io.write(ks->io.ctx, b, n);
}

/* Debug: global RX hex-dump flag */
static uint8_t g_kon_rxdbg = 0;
void kon_debug_rxdump(int on){ g_kon_rxdbg = on ? 1 : 0; }

void kon_printf(struct konsole *ks, const char *fmt, ...) {
#if KONSOLE_ENABLE_PRINTF
    va_list ap; va_start(ap, fmt);
    #if KONSOLE_USE_NPF
        extern int npf_vsnprintf(char *buffer, size_t bufsz, const char *format, va_list vlist);
        int n = npf_vsnprintf(ks->fmtbuf, sizeof(ks->fmtbuf), fmt, ap);
    #else
        int n = vsnprintf(ks->fmtbuf, sizeof(ks->fmtbuf), fmt, ap);
    #endif
    va_end(ap);
    if (n < 0) return;
    size_t len = (size_t)((n < (int)sizeof(ks->fmtbuf)) ? n : (int)sizeof(ks->fmtbuf));
    ts_write(ks, (const uint8_t*)ks->fmtbuf, len);
#else
    (void)ks; (void)fmt;
#endif
}

void kon_print_help(struct konsole *ks) {
    for (size_t i=0;i<ks->cmd_count;i++) {
        const char *h = ks->cmds[i].help ? ks->cmds[i].help : "";
        kon_printf(ks, "%-12s - %s\r\n", ks->cmds[i].name, h);
    }
}

void kon_clear_screen(struct konsole *ks) {
#if KONSOLE_ENABLE_VT100
    if (ks->mode == KON_MODE_ANSI && ks->vt100) {
        ts_puts(ks, "\x1b[2J\x1b[H");
        ks->last_screen_len = 0;
        return;
    }
#endif
    for (int i=0;i<30;i++) ts_puts(ks, "\r\n");
    ks->last_screen_len = 0;
}

static void execute_line(struct konsole *ks) {
    struct kon_line_state *ls = ks->line;

    ls->line[ls->len] = '\0';

    char line_copy[KONSOLE_MAX_LINE];
    size_t copy_len = (ls->len < (KONSOLE_MAX_LINE - 1)) ? ls->len : (KONSOLE_MAX_LINE - 1);
    memcpy(line_copy, ls->line, copy_len);
    line_copy[copy_len] = '\0';

    ts_puts(ks, "\r\n");

    char *argv[KONSOLE_MAX_ARGS];
    int argc = _kon_tokenize(line_copy, argv, KONSOLE_MAX_ARGS);

#if KONSOLE_HISTORY > 0
    if (argc > 0 && line_copy[0] != '\0') {
        _kon_line_add_history(ks, line_copy);
    }
#endif

    if (argc > 0) {
        const struct kon_cmd *cmd = _kon_find(ks->cmds, ks->cmd_count, argv[0]);
        if (cmd && cmd->fn) {
            cmd->fn(ks, argc, argv);
        } else {
            if (ks->on_unknown) ks->on_unknown(ks, line_copy);
            else kon_printf(ks, "unknown: %s\r\n", argv[0]);
        }
    }

    _kon_line_reset(ks);

#if KONSOLE_HISTORY > 0
    _kon_line_history(ks, +1);
#endif

    if (ks->prompt) _kon_line_redraw(ks);
}

void kon_feed(struct konsole *ks, const uint8_t *data, size_t len) {
    for (size_t i=0;i<len;i++) {
        if (g_kon_rxdbg) { char s[8]; snprintf(s,sizeof(s),"%02X ", data[i]); ts_puts(ks,s); }

        uint8_t c = data[i];

#if KONSOLE_EOL_MODE == 1
        /* Execute on CR or LF; if CRLF arrives, run once on CR and ignore the LF */
        if (c == '\r') { execute_line(ks); ks->saw_cr = 1; continue; }
        if (c == '\n') { if (ks->saw_cr) { ks->saw_cr = 0; continue; } execute_line(ks); continue; }
        ks->saw_cr = 0;
#else
        if (c == '\r') { ks->saw_cr = 1; continue; }
        if (c == '\n') { execute_line(ks); ks->saw_cr = 0; continue; }
        if (ks->saw_cr) { ks->saw_cr = 0; }
#endif

        if (_kon_line_handle_esc(ks, c)) continue;

        if (c == '\b' || c == 127) { _kon_line_backspace(ks); continue; }
        if (c == '\t') { _kon_line_insert(ks,' '); _kon_line_insert(ks,' '); continue; }
        if (c >= 32 && c < 127) { _kon_line_insert(ks,(char)c); continue; }
    }
}

void konsole_poll(struct konsole *ks) {
    uint8_t buf[64];
    size_t avail = ks->io.read_avail ? ks->io.read_avail(ks->io.ctx) : 1;
    if (!avail) return;
    size_t n = ks->io.read ? ks->io.read(ks->io.ctx, buf, sizeof(buf)) : 0;
    if (n) kon_feed(ks, buf, n);
}

void kon_banner(struct konsole *ks, const char *banner) {
    if (banner) { ts_puts(ks, banner); ts_puts(ks, "\r\n"); }
    if (ks->prompt) _kon_line_redraw(ks);
}

void konsole_init(struct konsole *ks, const struct konsole_io *io,
                  const struct kon_cmd *cmds, size_t cmd_count,
                  const char *prompt, bool vt100) {
    memset(ks, 0, sizeof(*ks));
    if (io) ks->io = *io;
    ks->cmds = cmds; ks->cmd_count = cmd_count;
    ks->prompt = prompt;
    ks->vt100 = vt100;
    ks->mode  = KON_MODE_ANSI; /* default safe mode */
    ks->line = (struct kon_line_state*)calloc(1, sizeof(*ks->line));
    _kon_line_reset(ks);
}

void konsole_init_with_storage(struct konsole *ks, struct kon_line_state *storage,
                               const struct konsole_io *io,
                               const struct kon_cmd *cmds, size_t cmd_count,
                               const char *prompt, bool vt100) {
    memset(ks, 0, sizeof(*ks));
    if (io) ks->io = *io;
    ks->cmds = cmds; ks->cmd_count = cmd_count;
    ks->prompt = prompt;
    ks->vt100 = vt100;
    ks->mode  = KON_MODE_ANSI; /* default safe mode */
    ks->line = storage;
    if (storage) memset(storage, 0, sizeof(*storage));
    _kon_line_reset(ks);
}

void konsole_set_mode(struct konsole *ks, kon_mode_t m){ ks->mode = m; }
kon_mode_t konsole_get_mode(struct konsole *ks){ return ks->mode; }
