# pico-bios-kbd

A 4-button layered USB HID keyboard built on a Raspberry Pi Pico W. Designed for headless BIOS navigation when you don't have a keyboard handy.

## Why

I needed to change the boot order on a ThinkCentre M920Q but didn't have a USB keyboard. So I built one with a Pico, a breadboard, and 4 tactile buttons.

## Features

- **2 layers** with 6 keys across 4 buttons
- **LED indicator** for current layer (off = Layer 0, on = Layer 1)
- **Hold-to-flash** — long press the layer button (2s) to enter bootloader mode
- **USB HID Boot Protocol** — works in BIOS, no drivers needed

## Key Layout

| Button | Layer 0 (LED off) | Layer 1 (LED on) |
| ------ | ----------------- | ---------------- |
| 1      | Layer toggle      | Layer toggle     |
| 2      | Up                | Down             |
| 3      | Left              | Right            |
| 4      | F12               | Enter            |

## Wiring

4 tactile buttons connected to GPIO pins with internal pull-ups. Each button has one side wired to a GPIO pin and the other to GND.

| Button | GPIO | Pico Pin |
| ------ | ---- | -------- |
| Layer  | GP0  | Pin 1    |
| Btn 1  | GP1  | Pin 2    |
| Btn 2  | GP2  | Pin 4    |
| Btn 3  | GP3  | Pin 5    |

GND available on pins 3, 8, 13, 18.

## Building

### Prerequisites

- [ARM GCC toolchain](https://developer.arm.com/downloads/-/gnu-rm) (`brew install --cask gcc-arm-embedded`)
- [CMake](https://cmake.org/) (`brew install cmake`)
- [picotool](https://github.com/raspberrypi/picotool) (`brew install picotool`)

The Pico SDK is fetched automatically by CMake during the first build.

### Build

```bash
cd firmware
mkdir build && cd build
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
make -j$(sysctl -n hw.ncpu)
```

### Flash

Hold BOOTSEL on the Pico and plug it in, then:

```bash
picotool load pico_keyboard.uf2 && picotool reboot
```

Once the firmware is running, you can re-enter bootloader mode by holding the layer button for 2 seconds. No need to unplug.

## Hardware

- Raspberry Pi Pico W
- 4x tactile push buttons (6mm)
- Breadboard + jumper wires

KiCad schematic files are in the `hardware/` directory.

## License

[MIT](LICENSE)
