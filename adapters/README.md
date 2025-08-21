# Konsole Adapters

Thin shims that connect **konsole** to different UART backends.

## Files
- `arduino_serial.cpp` — Arduino `HardwareSerial` (builds when `ARDUINO` is defined).
- `espidf_uart.c` — ESP-IDF UART driver (builds when `ESP_PLATFORM` and not `ARDUINO`).
- `stm32_hal_uart.c` — STM32 HAL UART (builds on STM32 family macros or `KONSOLE_ADAPTER_STM32`).
- `nrf5_app_uart.c` — Nordic nRF5 `app_uart` (builds when `NRF51/NRF52` or `KONSOLE_ADAPTER_NRF5`).

## Use
1. Ensure `src/konsole` is on your include path (e.g. PlatformIO: `build_flags = -Isrc/konsole`), or keep the relative includes as-is.
2. Initialize the adapter and pass its `struct konsole_io` to `konsole_init(...)`.
3. Call `konsole_poll(&ks);` regularly.

## Notes
- Build guards keep non-matching adapters out of your binary automatically.
- For STM32/nRF5, your app should include vendor headers and wire RX callbacks to the `*_on_rx()` helper.
