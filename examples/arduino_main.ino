#include <Arduino.h>
extern "C" {
#include "../konsole.h"
#include "../static.h"
}

static int cmd_echo(struct konsole *ks, int argc, char **argv) {
  for (int i=1;i<argc;i++) kon_printf(ks, "%s%s", argv[i], i+1<argc?" ":"\r\n");
  return 0;
}
static int cmd_sys(struct konsole *ks, int, char**) {
  kon_printf(ks, "uptime: %lu ms\r\n", (unsigned long)millis());
  return 0;
}
static int cmd_help(struct konsole *ks, int, char**) { kon_print_help(ks); return 0; }
static int cmd_clear(struct konsole *ks, int, char**) { kon_clear_screen(ks); return 0; }
static int cmd_rxdbg(struct konsole *ks, int argc, char** argv) {
  if (argc<2) { kon_printf(ks, "rxdbg: %s\r\n", "off|on"); return 0; }
  if (!strcmp(argv[1],"on"))  { kon_debug_rxdump(1); kon_printf(ks, "rxdbg on\r\n"); }
  else                        { kon_debug_rxdump(0); kon_printf(ks, "rxdbg off\r\n"); }
  return 0;
}
static int cmd_mode(struct konsole *ks, int argc, char** argv) {
  if (argc<2) {
    kon_printf(ks, "mode: %s\r\n", konsole_get_mode(ks)==KON_MODE_ANSI?"ansi":"compat");
    return 0;
  }
  if (!strcmp(argv[1],"ansi"))   konsole_set_mode(ks, KON_MODE_ANSI);
  else                           konsole_set_mode(ks, KON_MODE_COMPAT);
  kon_printf(ks, "mode: %s\r\n", konsole_get_mode(ks)==KON_MODE_ANSI?"ansi":"compat");
  return 0;
}

static const kon_cmd g_cmds[] = {
  {"help","list commands",  cmd_help},
  {"echo","echo arguments", cmd_echo},
  {"sys", "show system info", cmd_sys},
  {"clear","clear screen",  cmd_clear},
  {"rxdbg","toggle RX hex dump", cmd_rxdbg},
  {"mode","set console mode (compat|ansi)", cmd_mode},
};

static konsole g_ks;

void setup() {
  Serial.begin(115200);
  while(!Serial) {}

  konsole_io io = {
    [](void*)->size_t { return Serial.available(); },
    [](void*, uint8_t* b, size_t n)->size_t { size_t i=0; while (i<n && Serial.available()) b[i++]=Serial.read(); return i; },
    [](void*, const uint8_t* b, size_t n)->size_t { return Serial.write(b,n); },
    [](void*)->uint32_t { return (uint32_t)millis(); },
    nullptr
  };

  // Default is COMPAT mode; ANSI available via 'mode ansi'
  konsole_init(&g_ks, &io, g_cmds, sizeof(g_cmds)/sizeof(g_cmds[0]), "> ", /*vt100*/ true);
  kon_banner(&g_ks, "konsole ready");
}

void loop() {
  konsole_poll(&g_ks);
}
