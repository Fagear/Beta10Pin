# Beta10Pin

## What is this?

**Beta10Pin** is the hardware adapter that allows connection of Sony Beta camera with 14-pin (K-type) cable to EIAJ 10-pin (J-type) video tape recorder (VTR). Like **Sony CMA-1010** but more functional.

## Supported VTRs

The main target VTR is **Panasonic/National NV-180** which has a small footprint, full direct drive 4-head die-cast transport and non-EIAJ-standard serial link to a camera. But this adapter *should* work with other VTRs that have EIAJ 10-pin connector for a camera. Also, older **Panasonic/National NV-100** VTR and its derivatives have a serial link to the camera but its protocol is unknown and compatibility not tested.
<details>
<summary>NV-180 compatible machines</summary>

**Panasonic NV-180** had several upgraded models...

- **Panasonic AG-2400** (same NV-180 put in "professional" line of products)
- **Panasonic PV-8000** (upgraded model with stereo linear sound and more tape speeds)
- **Panasonic PV-9000** (upgraded yet again with Hi-Fi stereo sound)

...and rebranded variants from other manufacturers:

- **Bauer/Bosch VRP 30** (rebranded NV-180)
- **Blaupunkt RTX-260** (rebranded NV-180)
- **Canon VR-30** (rebranded PV-8000)
- **Canon VR-40** (rebranded PV-9000)
- **Curtis-Mathes KV-773** (rebranded PV-9000)
- **Grundig VS 120** (rebranded NV-180)
- **Magnavox Escort XD (VR8454)** (rebranded PV-8000)
- **Magnavox Escort XHD (VR8455)** (rebranded PV-9000)
- **Olympus VC-104** (rebranded NV-180)
- **Olympus VC-105** (rebranded PV-8000)
- **Olympus VC-106** (rebranded PV-9000)
- **Philips VR6711** (rebranded NV-180)
- **Quasar VP-5741XQ** (rebranded PV-8000)
- **Quasar VP-5748XE** (rebranded PV-9000)
- **Sylvania VC-4512** (rebranded PV-8000)

</details>

## Why?

Why not just use CMA-1010?

It's quite hard to find, it's pretty big, it has thick and stiff cable and does not support all features of Sony Beta cameras.

## Features

- Provides power from VTR to a Beta Camera
- Allows video and audio to be transfered from a camera to a VTR when VTR is in record mode
- Allows video from VTR to be displayed in a camera's viewfinder when VTR is in playback mode
- Converts level and pulse record commands from camera to level "unpause" signal for VTR
- Monitors input voltage and indicates low battery by blinking tally light in a camera
- Monitors camera's power consumption and puts VTR in pause and stand by to preserve energy
- Allows powering camera from USB-C PD/QC source
- Outputs video through RCA if not connected to a VTR through 10-pin EIAJ connector

## Firmware

Adapter utilizes **Atmel AVR** MCU for logic and voltage measurement.

Target MCU is **ATmega 48** (either variant). But firmware should be compilable and working on **ATmega 88/168/328** variants as well.

MCU can be clocked from internal 8 MHz RC-oscillator but **external 8 MHz Xtal** is recommended for precise timing.

Due to dependance on ADC measurements and proper connection with MCU should run on voltages from **4.5 V** to **5.0 V**.

<details>
<summary>AVR fuse information</summary>

Fuses for **ATmega48P** with *8 MHz Xtal*:
- **SUT0** = 0
- **CKSEL3** = 0
- **SPIEN** = 0
- **EESAVE** = 0
- all other at "1"

In hex form:
- low byte: **0xE7**
- high byte: **0xD7**
- extended byte: **0xFF**

For **ATmega328P** with *internal RC generator*:
- **SUT1** = 0
- **SUT0** = 0
- **CKSEL3** = 0
- **CKSEL2** = 0
- **CKSEL0** = 0
- **SPIEN** = 0
- **EESAVE** = 0
- **BODLEVEL1** = 0
- **BODLEVEL0** = 0
- all other at "1"

In hex form:
- low byte: **0xC2**
- high byte: **0xD7**
- extended byte: **0xFC**
</details>

## Current state

- Working basic control (VTR controls video path relay, camera controls pause line to VTR)
- Working voltage monitoring and indication in camera
- Working camera connection detection and NV-180 standby function operation
- Working serial link detection and RX/TX

## Plans

- Add state machine to operate NV-180 VTRs through serial link
- Test adapter with other VTRs (without serial link and standby control)

## License
Program copyright 2024.

This program is free software.
Licensed under the Apache License, Version 2.0 (the "License");