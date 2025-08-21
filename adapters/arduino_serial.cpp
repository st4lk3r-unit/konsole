#if defined(ARDUINO)

#include <Arduino.h>
#include "../konsole.h"
#include "../io.h"

struct KonArduinoCtx {
  HardwareSerial * serial;
};
static size_t ks_read_avail(void * ctx) {
  auto * c = (KonArduinoCtx * ) ctx;
  return (size_t) c -> serial -> available();
}
static size_t ks_read(void * ctx, uint8_t * buf, size_t len) {
  auto * c = (KonArduinoCtx * ) ctx;
  size_t i = 0;
  while (i < len && c -> serial -> available()) buf[i++] = (uint8_t) c -> serial -> read();
  return i;
}
static size_t ks_write(void * ctx,
  const uint8_t * buf, size_t len) {
  auto * c = (KonArduinoCtx * ) ctx;
  return (size_t) c -> serial -> write(buf, len);
}
static uint32_t ks_millis(void * ) {
  return (uint32_t) millis();
}
konsole_io arduino_serial_io(HardwareSerial * serial) {
  static KonArduinoCtx ctx;
  ctx.serial = serial;
  return (konsole_io) {
    ks_read_avail,
    ks_read,
    ks_write,
    ks_millis,
    & ctx
  };
}
konsole_io arduino_serial_io(HardwareSerial & serial) {
  return arduino_serial_io( & serial);
}
#endif