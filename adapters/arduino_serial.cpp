#if defined(ARDUINO)
#include <Arduino.h>
extern "C" {
#include "../konsole.h"
}
static size_t ar_avail(void *ctx){ (void)ctx; return Serial.available(); }
static size_t ar_read(void *ctx, uint8_t *b, size_t n){ (void)ctx; size_t i=0; while(i<n && Serial.available()) b[i++]=Serial.read(); return i; }
static size_t ar_write(void *ctx, const uint8_t *b, size_t n){ (void)ctx; return Serial.write(b,n); }
static uint32_t ar_millis(void *ctx){ (void)ctx; return millis(); }
extern "C" void konsole_init_arduino(struct konsole *ks,const struct kon_cmd *cmds,size_t n,const char *prompt,bool vt100){
  struct konsole_io io={ar_avail,ar_read,ar_write,ar_millis,NULL}; konsole_init(ks,&io,cmds,n,prompt,vt100);
}
#endif
