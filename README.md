[English](README.md) | [日本語](README.ja.md)

# UPDI Unbricker

## Overview

This board is a UPDI programmer for AVR Dx/Ex-series 7.5 V high-voltage (HV) programming and modern ATtiny-series (tinyAVR 0/1/2) 12 V HV programming.
It injects HV pulses to restore programming capability when the UPDI pin has been accidentally or deliberately reconfigured, preventing normal UPDI programming with a standard UPDI tool.

The UPDI section of the circuit uses the design from [UPDI Adapter for AE-CH9102F (Rev. 2)](https://github.com/k-takata/PCB_UPDI_for_AE-CH9102F), and it includes an automatic mode switch controlled by the RTS signal.

## Supported scope

| Item | Status | Notes |
|------|--------|-------|
| AVR Dx/Ex 7.5 V HV UPDI | Supported | Applies 7.5 V pulse to RESET pin |
| ATtiny 12 V HV UPDI | Supported | Applies 12 V pulse to UPDI pin |
| Normal UPDI programming | Supported | SerialUPDI is used as the programmer |
| Devices verified so far | AVR64DD28, ATtiny1604, ATtiny402 | As of README creation |

## Software used

* KiCad 10.0
* Arduino IDE 2.3.10
  - [megaTinyCore](https://github.com/SpenceKonde/megaTinyCore) 2.6.11

## Switch and jumper roles

This board switches operating modes using three switches and one jumper.

| Part | Role | Main setting |
|------|------|--------------|
| SW1 | Enables/disables HV pulse generation | OFF = safe mode, ON = HV programming enabled |
| SW2 | Selects target supply voltage | Choose 5 V or 3.3 V |
| SW3 | Enables/disables power output from this board to the target | ON = power supplied, OFF = avoids double-powering when externally powered |
| J2 | Selects programming target | FW = firmware write, Target = target programming |

Set J2 to the FW side for firmware programming and the Target side for UPDI programming.
In the FW state, your PC connects to U4 to program it. In the Target state, it connects to the target device for UPDI communication and HV programming.

## Quick start

Before first use, write the firmware to the onboard MCU (U4).

1. Set J2 to the **FW** side.
2. Open `src/updi_unbricker/updi_unbricker.ino` in Arduino IDE and select the appropriate board and chip.
3. Set the programmer to SerialUPDI and write the firmware to U4.
4. After programming, return J2 to the **Target** side.
5. Connect either J5 or J6 according to the target device, and configure SW2 and SW3 as needed. (Set SW3 to ON if the board should supply power to the target.)
6. Turn SW1 ON to enable the HV circuit, then press the START button to inject the HV pulse.
7. In Arduino IDE, set the programmer to SerialUPDI and perform normal programming.

See “Usage” for detailed procedures.

## Safety precautions

* The UPDI pin on J5 outputs a 12 V HV pulse. The RESET pin on J6 outputs a 7.5 V HV pulse. Always verify the target circuit's voltage tolerance and protection measures.
* If the target is powered from an external source, set SW3 to OFF to avoid double-powering.
* When running HV, it is recommended to leave J3 disconnected.
* For the first test, avoid the complete target board if possible; use a bare chip or a minimal setup instead.

## Schematic

[![schema](images/schema.png)](images/schema.pdf)

## PCB layout

![PCB pattern](images/pcb-pattern.png)

## BOM

| Reference | Quantity | Value | Description |
|-----------|----------|-------|-------------|
| C1, C2 | 2 | 10 μF | 3225M or 3216M |
| C3, C7 | 2 | 0.1 μF | |
| C4 | 1 | 100 μF | ≥ 47 μF, ≥ 10 V |
| C5 | 1 | 47 μF | ≥ 47 μF, ≥ 25 V |
| C6 | 1 | 220 pF | 150 pF to 470 pF |
| D1, D3-D6 | 5 | [BAT43](https://akizukidenshi.com/catalog/g/g113907/) | Schottky barrier diode; other examples: [SD103A](https://akizukidenshi.com/catalog/g/g104271), [11EQS03L](https://akizukidenshi.com/catalog/g/g108997/) |
| D2 | 1 |  | 5.6 V Zener diode for overvoltage protection |
| D7 | 1 |  | 3.6 V Zener diode; adjust value as needed for LED (3.0 V to 3.6 V) |
| D8 | 1 | [OSRGHC5B32A](https://akizukidenshi.com/catalog/g/g106314/) | 2-color LED (red/green), common cathode, φ 5 mm, VDD voltage indicator |
| D9 | 1 | [1N4737A](https://www.sengoku.co.jp/mod/sgk_cart/detail.php?code=EEHD-0FMV) | 7.5 V Zener diode (\*1) |
| D10 | 1 | Optional | Zener diode (\*1) |
| D11 | 1 | Optional | Zener diode (\*1) |
| F1 | 1 | [MF-NSMF050-2](https://akizukidenshi.com/catalog/g/g115300/) | Resettable fuse, 0.5 A |
| J1 | 1 | [5077CR-16-SMC2-BK-TR](https://akizukidenshi.com/catalog/g/g114356/) | USB Type-C receptacle |
| J2 | 1 |  | 1 × 3 pin header for selecting the programming target |
| J3 | 1 |  | [L-shaped 1 × 6 pin socket](https://akizukidenshi.com/catalog/g/g109862/) for TTL serial connection |
| J5 | 1 |  | 1 × 3 pin socket for ATtiny UPDI connection |
| J4 | 1 |  | 1 × 3 pin header for RTS/DTR switching |
| J6 | 1 |  | 1 × 4 pin socket for AVR Dx/Ex UPDI connection |
| J7 | 1 |  | 1 × 4 pin header for firmware programming (\*2) |
| L1 | 1 | 330 μH | 100 μH to 330 μH, ≥ 100 mA, e.g. [AL0307-331K](https://akizukidenshi.com/catalog/g/g103968/) |
| Q1, Q3, Q6, Q8 | 4 | [BSS138](https://akizukidenshi.com/catalog/g/g104232/) | N-ch MOSFET |
| Q2, Q5, Q7 | 3 | [BSS84](https://akizukidenshi.com/catalog/g/g104269/) | P-ch MOSFET |
| Q4 | 1 |[SSM3J332R](https://akizukidenshi.com/catalog/g/g115985/)| P-ch MOSFET |
| R1, R15 | 2 | 470 Ω | 1/4 W, yellow-violet-brown-gold |
| R2, R3 | 2 | 5.1 kΩ | 1608M |
| R4, R17, R19 | 3 | 100 kΩ | 1/4 W, brown-black-gold-gold (10 kΩ to 100 kΩ) |
| R5 | 1 | 220 Ω | 1/4 W, red-red-brown-gold |
| R6 | 1 | 2.2 Ω | 1/4 W, red-red-gold-gold |
| R7, R8, R16, R18 | 4 | 10 kΩ | 1/4 W, brown-black-orange-gold |
| R9 | 1 | 200 Ω | 1/4 W, red-black-brown-gold (150 Ω to 220 Ω) |
| R10 | 1 | 56 kΩ 1% | 1/4 W, green-blue-black-red-brown (\*3) |
| R11 | 1 | 39 kΩ 1% | 1/4 W, orange-white-black-red-brown (\*3) |
| R12 | 1 | 2.7 kΩ 1% | 1/4 W, red-violet-black-brown-brown (\*3) |
| R13 | 1 | 200 Ω | 1/4 W, red-black-brown-gold; adjust according to LED (47 Ω to 220 Ω) |
| R14 | 1 | 470 Ω | 1/4 W, yellow-violet-brown-gold; adjust according to LED |
| R20 | (1) | 10 kΩ | Do not install (1/6 W, brown-black-orange-gold)|
| SW1-SW3 | 3 | [SS-12D00G3](https://akizukidenshi.com/catalog/g/g115707/) | Slide switch, SPDT, PCB mount |
| SW4 | 1 |  | Push button switch |
| U1 | 1 | [LM1117GS-3.3](https://akizukidenshi.com/catalog/g/g116989/) | |
| U2 | 1 | [CH340K](https://akizukidenshi.com/catalog/g/g116306/) | |
| U3 | 1 | [MC34063AN](https://akizukidenshi.com/catalog/g/g112016/) | (\*4) |
| U4 | 1 | [ATtiny402-SS](https://akizukidenshi.com/catalog/g/g130009/) | (\*5) |

(\*1) Use D9 alone, or use D10 + D11 together. Adjust so that the TP1 voltage is 7.5 V (Vdd + 2.0 V minimum, up to 8.5 V). For example, a combination such as 3.0 V + 5.1 V is also acceptable.  
(\*2) It is acceptable not to mount the pin header and instead use pogo pins or similar.  
(\*3) R10 and R11 are connected in parallel. Adjust the ratio of the equivalent resistance of R10 + R11 to the value of R12 to be about 8.6. Another recommended combination is R10 = 13 kΩ, R11 unpopulated, and R12 = 1.5 kΩ.  
(\*4) Choose one of: mount the [DIP version](https://akizukidenshi.com/catalog/g/g112016/), install the [SOP8 version](https://akizukidenshi.com/catalog/g/g117573/) directly, or mount through a [SOP8 adapter board](https://akizukidenshi.com/catalog/g/g105154).  
(\*5) Choose one of: mount ATtiny402 directly or through a [SOP8 adapter board](https://akizukidenshi.com/catalog/g/g105154).  

## About UPDI HV programming

UPDI (Unified Program and Debug Interface) is a programming method used in newer AVR devices.

UPDI uses a dedicated pin, but you can reconfigure the UPDI pin as a GPIO through fuse settings. However, if UPDI is disabled, the next UPDI programming attempt requires a special method to re-enable it. That method is high-voltage (HV) programming.

There are two types of HV programming. One is the 12 V pulse applied to the UPDI pin used in ATtiny devices, and the other is the 7.5 V pulse applied to the RESET pin used in AVR Dx/Ex devices.

1. ATtiny series:  
   Apply a 12 V pulse to the UPDI pin within 8.8 ms after power-on reset (POR), with a width of 100 μs to 1 ms.  
   If the pulse is not applied within the required time after POR, the pin function may interfere with the intended operation.
2. AVR Dx/Ex series:  
   Apply a 7.5 V pulse to the RESET pin for at least 10 μs, then send the NVMPROG key within 65 ms.  
   If transmission of the NVMPROG key is not completed in time, a reset is triggered automatically.  
   Unlike ATtiny devices, AVR Dx/Ex series have a separate RESET pin, and it cannot be used as an output pin; there are no POR-to-HV-pulse timing restrictions.

For safety, this board uses separate connectors for ATtiny and AVR Dx/Ex devices.
For simplified processing, this board resets both ATtiny and AVR Dx/Ex devices with POR, then applies an HV pulse, and finally sends the NVMPROG key. POR is unnecessary for AVR Dx/Ex, and sending the NVMPROG key is unnecessary for ATtiny, but it should be harmless.

**Reference material:**

* [ATtiny202/204/402/404/406 Data Sheet](https://ww1.microchip.com/downloads/aemDocuments/documents/MCU08/ProductDocuments/DataSheets/ATtiny202-204-402-404-406-DataSheet-DS40002318A.pdf) [PDF]  
  See “30. UPDI - Unified Program and Debug Interface” and “33. Electrical Characteristics” (33.18. UPDI Timing).
* [AVR64DD32/28 Datasheet](https://ww1.microchip.com/downloads/aemDocuments/documents/MCU08/ProductDocuments/DataSheets/AVR64DD32-28-Complete-DataSheet-DS40002315.pdf) [PDF]  
  See “34. UPDI - Unified Program and Debug Interface” and “36. Electrical Characteristics” (36.18. UPDI).

## Devices verified

* AVR64DD28
* ATtiny1604
* ATtiny402

## Usage

### Connections

Connect J1 to the PC.

Connect the target using J5 or J6.
J5 is the connector for ATtiny devices.

| Pin | Function |
|-----|----------|
| 1 | VDD (5 V / 3.3 V) |
| 2 | GND |
| 3 | UPDI (max 12 V) |

The UPDI pin outputs a 12 V HV pulse, so the target circuit must be designed to tolerate this voltage.

J6 is the connector for AVR Dx/Ex devices and matches the UPDI v2 connector used by the [AVR Programming Adapter](https://www.microchip.com/en-us/development-tool/AC31S18A).

| Pin | Function | Color |
|-----|----------|-------|
| 1 | RESET (max 7.5 V) | White |
| 2 | VDD (5 V / 3.3 V) | Red |
| 3 | GND | Black |
| 4 | UPDI | Green |

The RESET pin outputs a 7.5 V HV pulse, so the target circuit must be designed to tolerate this voltage.

J3 is a standard 6-pin TTL serial connector. You can select pin 6 on J3 as either RTS or DTR by inserting a jumper on J4.

| Pin | Function | Color |
|-----|----------|-------|
| 1 | GND | Black |
| 2 | CTS | Brown |
| 3 | VDD (5 V / 3.3 V) | Red |
| 4 | TxD | Orange |
| 5 | RxD | Yellow |
| 6 | RTS / DTR | Green |

SW1 selects whether HV programming is enabled. For safety, it is normally recommended to keep it OFF.

SW2 selects the target supply voltage. You can select 5 V or 3.3 V.

SW3 selects whether the board supplies power to the target. If set to ON, it supplies power; if set to OFF, it does not. If the target is already powered by another path, set this to OFF.

To power the target with this board, select the supply voltage with SW2 and set SW3 to ON. The LED color changes with the supply voltage: green at 3.3 V and orange (green + red) at 5 V. If SW3 is OFF and the target is not connected, the LED is turned off. If the target is connected and powered externally, the LED lights from the target supply.

### Firmware programming

#### Normal programming

To program the firmware to U4, configure and connect the switches and connectors as follows.

| Part | Setting / connection |
|------|---------------------|
| SW1 | Any |
| SW2 | Any |
| SW3 | Any |
| J1 | Connect PC |
| J2 | **FW** |
| J3 | Disconnected |
| J4 | Any |
| J5 | Disconnected |
| J6 | Disconnected |
| J7 | Disconnected |

Open the sketch in the `src` directory in Arduino IDE.  
Select the appropriate board and chip for the AVR mounted on U4.

| Part | Board | Chip |
|------|-------|------|
| U4 | megaTinyCore → ATtiny412/402/212/202 | ATtiny402 |

ATtiny402 has been used for verification, but other 8-pin ATtiny devices may also work.

Set the programmer to SerialUPDI to write the firmware.

#### HV programming for firmware recovery

If the UPDI pin of U4 has been disabled and you need to re-enable it via HV programming, you can recover the board by connecting another HV programmer to J7.

| Part | Setting / connection |
|------|---------------------|
| SW1 | Any |
| SW2 | Any |
| SW3 | Any |
| J1 | Disconnected |
| J2 | **Remove jumper** |
| J3 | Disconnected |
| J4 | Any |
| J5 | Disconnected |
| J6 | Disconnected |
| J7 | Connect another HV programmer |

To avoid effects from the 12 V pulse, remove the J2 jumper or set it to the Target side.  
Also disconnect all other connectors except J7.

### UPDI programming

#### Normal mode

Configure and connect the switches and connectors as follows.

| Part | Setting / connection |
|------|----------------------|
| SW1 | OFF for safety |
| SW2 | Select 5 V or 3.3 V according to the target |
| SW3 | Normally ON |
| J1 | Connect PC |
| J2 | **Target** |
| J3 | Connected or disconnected |
| J4 | Any |
| J5 / J6 | Connect the appropriate target connector |
| J7 | Disconnected |

SW3 is normally set to ON, but if the target is powered from another source, set it to OFF.  
J3 can remain connected without issue. The UPDI mode and serial communication mode are automatically switched by the RTS signal.

In the Arduino IDE, set the programmer to SerialUPDI, and programming can proceed normally.

#### HV programming

Configure and connect the switches and connectors as follows.

| Part | Setting / connection |
|------|----------------------|
| SW1 | **ON** |
| SW2 | Select 5 V or 3.3 V according to the target |
| SW3 | Normally ON |
| J1 | Connect PC |
| J2 | **Target** |
| J3 | Disconnected |
| J4 | Any |
| J5 / J6 | Connect the appropriate target connector |
| J7 | Disconnected |

SW2 is normally set to ON. For ATtiny targets in particular, keep it ON so this board supplies power.  
Connect either J5 or J6, depending on the target. Use J5 for ATtiny and J6 for AVR Dx/Ex. Connecting J5/J6 to the target circuit is fine as long as it supports HV, but connecting a bare chip is safer.  
J3 should remain disconnected.

With the connection set, press the START button. The board resets the target, sends the HV pulse and NVMPROG key, enabling UPDI. Then follow the usual Arduino IDE programming procedure; the device should be ready for programming.

### Serial communication

Configure and connect the switches and connectors as follows.

| Part | Setting / connection |
|------|----------------------|
| SW1 | OFF for safety |
| SW2 | Select 5 V or 3.3 V according to the target |
| SW3 | Normally ON |
| J1 | Connect PC |
| J2 | **Target** |
| J3 | Disconnected |
| J4 | Select according to purpose |
| J5 / J6 | Connected or disconnected |
| J7 | Disconnected |

J5/J6 may be connected without issue. The UPDI mode and serial communication mode are automatically switched by the RTS signal.  
You can select pin 6 on J3 as RTS or DTR by inserting a jumper on J4.

## AVRDUDE usage examples

Here is an example of setting fuses using [AVRDUDE](https://github.com/avrdudes/avrdude/). AVRDUDE requires v7.0 or later with SerialUPDI support.

The programmer is specified with `-c serialupdi`.

### AVR64DD28

For AVR64DD28, the UPDI pin setting is controlled by the UPDIPINCFG bit in the SYSCFG0 fuse.

Use `-p 64dd28` to specify the device name.

#### Reading the fuse

To read SYSCFG0, specify `-U fuse5:r:-:h`.
`fuse5` indicates SYSCFG0, `r` means read, `-` means standard output, and `h` means hexadecimal output.

```
>avrdude -c serialupdi -p 64dd28 -P COM8 -b 115200 -v -U fuse5:r:-:h
Avrdude version 8.1
Copyright see https://github.com/avrdudes/avrdude/blob/main/AUTHORS

System wide configuration file is C:\Users\<USER>\AppData\Local\Arduino15\packages\DxCore\tools\avrdude\avrdude_v8.1-released\etc\avrdude.conf

Using port            : COM8
Using programmer      : serialupdi
Setting baud rate     : 115200
AVR part              : AVR64DD28
Programming modes     : SPM, UPDI
Programmer type       : serialupdi
Description           : SerialUPDI
NVM type 2: 24-bit, word oriented write
Entering NVM programming mode
Chip silicon revision: 1.3

AVR device initialized and ready to accept instructions
Device signature = 1E 96 1B (AVR64DD28)
Reading fuse5/syscfg0 memory ...
Writing 1 byte to output file <stdout>
0xd9
Leaving NVM programming mode

Avrdude done.  Thank you.
```

The default value of SYSCFG0 is 0xd0, but it is currently set to 0xd9.

#### Change the UPDI pin to GPIO

Change the UPDIPINCFG bit (Bit 4) from 1 to 0 to switch the UPDI pin to GPIO.
Use `-U fuse5:w:0xc9:m` to set SYSCFG0 to 0xc9. `w` means write, and `m` means immediate value.

```
>avrdude -c serialupdi -p 64dd28 -P COM8 -b 115200 -v -U fuse5:w:0xc9:m
Avrdude version 8.1
Copyright see https://github.com/avrdudes/avrdude/blob/main/AUTHORS

System wide configuration file is C:\Users\<USER>\AppData\Local\Arduino15\packages\DxCore\tools\avrdude\avrdude_v8.1-released\etc\avrdude.conf

Using port            : COM8
Using programmer      : serialupdi
Setting baud rate     : 115200
AVR part              : AVR64DD28
Programming modes     : SPM, UPDI
Programmer type       : serialupdi
Description           : SerialUPDI
NVM type 2: 24-bit, word oriented write
Entering NVM programming mode
Chip silicon revision: 1.3

AVR device initialized and ready to accept instructions
Device signature = 1E 96 1B (AVR64DD28)
Reading 1 byte for fuse5/syscfg0 from input file 0xc9
in 1 section [0, 0]
Writing 1 byte (0xC9) to fuse5/syscfg0, 1 byte written, 1 verified
Leaving NVM programming mode

Avrdude done.  Thank you.
```

#### Reading the fuse again

Read SYSCFG0 again. Since the UPDI pin has been changed to GPIO, the UPDI link cannot be established and the read fails.

```
>avrdude -c serialupdi -p 64dd28 -P COM8 -b 115200 -v -U fuse5:r:-:h
Avrdude version 8.1
Copyright see https://github.com/avrdudes/avrdude/blob/main/AUTHORS

System wide configuration file is C:\Users\<USER>\AppData\Local\Arduino15\packages\DxCore\tools\avrdude\avrdude_v8.1-released\etc\avrdude.conf

Using port            : COM8
Using programmer      : serialupdi
Setting baud rate     : 115200
AVR part              : AVR64DD28
Programming modes     : SPM, UPDI
Programmer type       : serialupdi
Description           : SerialUPDI
Error: UPDI link initialization failed
Error: initialization failed  (rc = -1)
 - double check the connections and try again
 - use -b to set lower baud rate, e.g. -b 57600
 - use -F to override this check
Leaving NVM programming mode

Avrdude done.  Thank you.
```

#### Press START to restore the UPDI pin

Pressing the START button temporarily makes UPDI usable again.
With that state, change the UPDIPINCFG bit back to 1 and restore the UPDI pin.

Use `-U fuse5:w:0xd9:m` to set SYSCFG0 back to 0xd9.

```
>avrdude -c serialupdi -p 64dd28 -P COM8 -b 115200 -v -U fuse5:w:0xd9:m
Avrdude version 8.1
Copyright see https://github.com/avrdudes/avrdude/blob/main/AUTHORS

System wide configuration file is C:\Users\<USER>\AppData\Local\Arduino15\packages\DxCore\tools\avrdude\avrdude_v8.1-released\etc\avrdude.conf

Using port            : COM8
Using programmer      : serialupdi
Setting baud rate     : 115200
AVR part              : AVR64DD28
Programming modes     : SPM, UPDI
Programmer type       : serialupdi
Description           : SerialUPDI
Device in NVM programming state, leaving programming mode
Device in reset status, trying to release it
NVM type 2: 24-bit, word oriented write
Entering NVM programming mode
Chip silicon revision: 1.3

AVR device initialized and ready to accept instructions
Device signature = 1E 96 1B (AVR64DD28)
Reading 1 byte for fuse5/syscfg0 from input file 0xd9
in 1 section [0, 0]
Writing 1 byte (0xD9) to fuse5/syscfg0, 1 byte written, 1 verified
Leaving NVM programming mode

Avrdude done.  Thank you.
```

### ATtiny1604

For ATtiny1604, the UPDI pin setting is controlled by the RSTPINCFG bits in the SYSCFG0 fuse.

Use `-p t1604` to specify the device name.

#### Reading the fuse

To read SYSCFG0, specify `-U fuse5:r:-:h`.
`fuse5` indicates SYSCFG0, `r` means read, `-` means standard output, and `h` means hexadecimal output.

```
>avrdude -c serialupdi -p t1604 -P COM8 -b 115200 -v -U fuse5:r:-:h
Avrdude version 8.1
Copyright see https://github.com/avrdudes/avrdude/blob/main/AUTHORS

System wide configuration file is C:\Users\<USER>\AppData\Local\Arduino15\packages\DxCore\tools\avrdude\avrdude_v8.1-released\etc\avrdude.conf

Using port            : COM8
Using programmer      : serialupdi
Setting baud rate     : 115200
AVR part              : ATtiny1604
Programming modes     : SPM, UPDI
Programmer type       : serialupdi
Description           : SerialUPDI
NVM type 0: 16-bit, page oriented write
Entering NVM programming mode
Chip silicon revision: 0.0

AVR device initialized and ready to accept instructions
Device signature = 1E 94 25 (ATtiny1604)
Reading fuse5/syscfg0 memory ...
Writing 1 byte to output file <stdout>
0xf6
Leaving NVM programming mode

Avrdude done.  Thank you.
```

The default value of SYSCFG0 is 0xf6.

#### Change the UPDI pin to GPIO

Change the RSTPINCFG bits (Bit 3:2) from 1 to 0 to switch the UPDI pin to GPIO.
Use `-U fuse5:w:0xf2:m` to set SYSCFG0 to 0xf2. `w` means write, and `m` means immediate value.

Note that if you change the RSTPINCFG bits to 2 (SYSCFG0: 0xfa), the UPDI pin becomes the RESET pin.

```
>avrdude -c serialupdi -p t1604 -P COM8 -b 115200 -v -U fuse5:w:0xf2:m
Avrdude version 8.1
Copyright see https://github.com/avrdudes/avrdude/blob/main/AUTHORS

System wide configuration file is C:\Users\<USER>\AppData\Local\Arduino15\packages\DxCore\tools\avrdude\avrdude_v8.1-released\etc\avrdude.conf

Using port            : COM8
Using programmer      : serialupdi
Setting baud rate     : 115200
AVR part              : ATtiny1604
Programming modes     : SPM, UPDI
Programmer type       : serialupdi
Description           : SerialUPDI
NVM type 0: 16-bit, page oriented write
Entering NVM programming mode
Chip silicon revision: 0.0

AVR device initialized and ready to accept instructions
Device signature = 1E 94 25 (ATtiny1604)
Reading 1 byte for fuse5/syscfg0 from input file 0xf2
in 1 section [0, 0]
Writing 1 byte (0xF2) to fuse5/syscfg0, 1 byte written, 1 verified
Leaving NVM programming mode

Avrdude done.  Thank you.
```

#### Reading the fuse again

Read SYSCFG0 again. Since the UPDI pin has been changed to GPIO, the UPDI link cannot be established, and the read fails.

```
>avrdude -c serialupdi -p t1604 -P COM8 -b 115200 -v -U fuse5:r:-:h
Avrdude version 8.1
Copyright see https://github.com/avrdudes/avrdude/blob/main/AUTHORS

System wide configuration file is C:\Users\<USER>\AppData\Local\Arduino15\packages\DxCore\tools\avrdude\avrdude_v8.1-released\etc\avrdude.conf

Using port            : COM8
Using programmer      : serialupdi
Setting baud rate     : 115200
AVR part              : ATtiny1604
Programming modes     : SPM, UPDI
Programmer type       : serialupdi
Description           : SerialUPDI
Error: UPDI link initialization failed
Error: initialization failed  (rc = -1)
 - double check the connections and try again
 - use -b to set lower baud rate, e.g. -b 57600
 - use -F to override this check
Leaving NVM programming mode

Avrdude done.  Thank you.
```

#### Press START to restore the UPDI pin

Pressing the START button temporarily makes UPDI usable again.
With that state, change the RSTPINCFG bits back to 1 and restore the UPDI pin.

Use `-U fuse5:w:0xf6:m` to set SYSCFG0 back to 0xf6.

```
>avrdude -c serialupdi -p t1604 -P COM8 -b 115200 -v -U fuse5:w:0xf6:m
Avrdude version 8.1
Copyright see https://github.com/avrdudes/avrdude/blob/main/AUTHORS

System wide configuration file is C:\Users\<USER>\AppData\Local\Arduino15\packages\DxCore\tools\avrdude\avrdude_v8.1-released\etc\avrdude.conf

Using port            : COM8
Using programmer      : serialupdi
Setting baud rate     : 115200
AVR part              : ATtiny1604
Programming modes     : SPM, UPDI
Programmer type       : serialupdi
Description           : SerialUPDI
NVM type 0: 16-bit, page oriented write
Entering NVM programming mode
Chip silicon revision: 0.0

AVR device initialized and ready to accept instructions
Device signature = 1E 94 25 (ATtiny1604)
Reading 1 byte for fuse5/syscfg0 from input file 0xf6
in 1 section [0, 0]
Writing 1 byte (0xF6) to fuse5/syscfg0, 1 byte written, 1 verified
Leaving NVM programming mode

Avrdude done.  Thank you.
```

## Troubleshooting

### Cannot program even after HV

* Confirm that J2 is set to the **Target** side.
* Confirm that the programmer is set to SerialUPDI.
* Do not wait too long after pressing START before starting the programming operation.

### Target does not start or is unstable

* Check the SW2 and SW3 settings and ensure there is no double-powering from an external source.
* Check that GND is common.
* Re-check the orientation and pin mapping of J5 and J6.

### Concern about the HV voltage

* Adjust R10, R11, and R12 so that the TP2 voltage is around 12 V (11.5 V to 12.5 V).
* Adjust D5 or D6 + D7 so that the TP3 voltage is around 7.5 V (Vdd + 2.0 V minimum, up to 8.5 V).
* For the first evaluation, use a bare chip or a minimal configuration and confirm that it works before connecting to the actual device.

## Circuit description

### 12 V generation circuit

An MC34063A generates 12 V. This IC can switch up to 1.5 A, but we don't need that much current here, so the design is configured to output roughly 20 mA.

In the R10, R11, and R12 divider circuit, we adjusted the ratio of the equivalent resistance of R10 + R11 to R12 to about 8.6. We used the 56 kΩ, 39 kΩ, and 2.7 kΩ combination because it was the closest match available in the 1% resistor kit we had, but a typical combination is 13 kΩ and 1.5 kΩ.

The output voltage can be approximated with this equation:

$$
V_{out} = 1.25 \times \left(1 + \frac{R_{top}}{R_{bottom}}\right)
$$

Here, $R_{top}$ is the equivalent resistance of R10 and R11, and $R_{bottom}$ is R12. In practice, adjust the divider ratio to about 8.6 so TP2 is around 12 V.

Example combinations:

| R10 | R11 | R12  | $V_{out}$ | Note |
|-----|-----|------|-----------|------|
| 56 k | 39 k | 2.7 k | 11.89     | E12 series |
| 13 k | -    | 1.5 k | 12.08     | E24 series |
| 11 k | -    | 1.3 k | 11.83     | E24 series |
| 33 k | -    | 3.9 k | 11.83     | E12 series |
| 18 k |100 k | 1.8 k | 11.84     | E12 series |
| 51 k | 51 k | 3.0 k | 11.88     | E24 series |
| 43 k |680 k | 4.7 k | 12.01     | E24 series |
| 75 k |470 k | 7.5 k | 12.03     | E24 series |


### CH340K 3.3 V operation

When using CH340K at 3.3 V, the datasheet says to connect the V3 pin to the VCC pin. However, if no current is drawn from the V3 pin, it appears to work fine to connect only a 0.1 μF capacitor.

Reference: [Non-Compliant Use of CH340 V3 Pin: Deep Dive for Engineers](https://www.digikey.com/en/maker/blogs/2025/non-compliant-use-of-ch340-v3-pin-deep-dive-for-engineers)

### Automatic switching between UPDI mode and serial communication mode

As described above, the UPDI section uses the circuit from [UPDI Adapter for AE-CH9102F (Rev. 2)](https://github.com/k-takata/PCB_UPDI_for_AE-CH9102F), which includes automatic mode switching using the RTS signal.

In general, a two-pole, double-throw switch is often used to switch between UPDI mode and serial communication mode. Here, the TxD line remains connected, allowing switching with a single-pole, double-throw switch. In addition, to keep the switching circuit compact, a discrete-component implementation is used instead of an analog switch IC such as 4052.

### VDD voltage indicator

The LED D8 color indicates the VDD voltage of the target. It is green for 3.3 V and orange (green + red) for 5.0 V.
To reduce component count, the design uses only a Zener diode and two resistors.
If the LED is changed, the Zener voltage and resistor values must be adjusted to match the LED forward voltage; otherwise, the LED will not light cleanly. Another point to consider is that 3.3 V results in a slightly dimmer LED than 5.0 V.

## Issues with DxCore 1.6.2

### Critical issue

The latest DxCore version at the time of writing, 1.6.2, had a critical problem that made it unusable with AVR DD-series devices.
Two issues exist: one causes an error that prevents programming, and the other sets the fuse incorrectly and disables the UPDI pin.

* [On 1.6.2 upload to AVR64DD14 fails with prog.py: error: unrecognized arguments · Issue #629 · SpenceKonde/DxCore](https://github.com/SpenceKonde/DxCore/issues/629)
* [Add missing zero-bit in SYSCFG0 for DD-chips by felias-fogg · Pull Request #638 · SpenceKonde/DxCore](https://github.com/SpenceKonde/DxCore/pull/638)

If only the first issue is fixed and programming is attempted, the second issue makes UPDI programming impossible. This project was created specifically to address this issue.

If both issues are fixed, AVR DD-series devices can work with DxCore 1.6.2.

Specifically, on Windows, move to `C:\Users\<USERNAME>\AppData\Local\Arduino15\packages\DxCore\hardware\megaavr\1.6.2` and change `boards.txt` as follows.

```diff
--- boards.txt.orig
+++ boards.txt
@@ -1209,7 +1209,7 @@
 avrdd.bootloader.wdttimeotbits=0000
 avrdd.bootloader.BODCFG=0b{bootloader.bodlevbits}{bootloader.bodmodebits}
 avrdd.bootloader.updipinbit=1
-avrdd.bootloader.SYSCFG0=0b110{bootloader.updipinbit}{bootloader.resetpinbit}0{bootloader.eesavebit}
+avrdd.bootloader.SYSCFG0=0b110{bootloader.updipinbit}{bootloader.resetpinbit}00{bootloader.eesavebit}
 avrdd.bootloader.SYSCFG1=0b000{bootloader.mviobits}{bootloader.sutbits}
 avrdd.bootloader.CODESIZE=0x00
 avrdd.bootloader.BOOTSIZE=0x01
@@ -1227,8 +1227,8 @@
 avrdd.upload.maximum_data_size=0
 # The maximum size and data size attributes are overridden by the selected chip. If you are avoiding specifying that somehow, there is no hope of anything working, so don't do that.
 # Each top-level entry supports at least a dozen parts with varying memory constraints.
-avrdd.program.serupdifuse5="-Ufuse5:w:{bootloader.SYSCFG0}:m"
-avrdd.program.avrdudefuse5=5:{bootloader.SYSCFG0}
+avrdd.program.avrdudefuse5="-Ufuse5:w:{bootloader.SYSCFG0}:m"
+avrdd.program.serupdifuse5=5:{bootloader.SYSCFG0}
 
 
 #----------------------------------------#
```

This fix is overwritten when DxCore is updated or reinstalled via the Board Manager, so it must be reapplied in such cases.

If new functions in DxCore 1.6.x are needed (for example, support for AVR DU series), use this workaround until a fixed release becomes available. If those new features are not needed, using DxCore 1.5.11 is recommended.

### Other issues

Besides the critical writing problem in DxCore 1.6.2, several additional issues have been found.

One of the most significant appears to be that `analogReference()` does not work, causing `analogRead()` to fail. See [fix analogReference() by wke67 · Pull Request #643 · SpenceKonde/DxCore](https://github.com/SpenceKonde/DxCore/pull/643) for details.

## Finished product

[![Finished product](images/unbricker-thumb.jpg)](images/unbricker.jpg)

## License

CC0

## Reference projects

Links to related projects and reference information for HV UPDI programming.

### ATtiny only

There are many projects that support 12 V HV programming. The following are a few examples. (However, not many projects enforce the required timing to apply the HV pulse within the allowed time window after POR. Someone calls it Power Cycle HV (PCHV).)

* [todopapa/UPDI_HV_WRITER-w-RESET: This is a new AVR ATTINY series UPDI programmer with HV pulse injection avility on power on reset timing.](https://github.com/todopapa/UPDI_HV_WRITER-w-RESET)
* [DIY Arduino Nano HV UPDI Programmer - Electronics-Lab](https://www.electronics-lab.com/diy-arduino-nano-hv-updi-programmer/)
* [UPDI HVP のための 12V を得る方法の試行 | シャポログ](https://blog.shapoco.net/2025/0308-updi-hvp-with-ae-ch340e/)
* [Dlloydev/Updi-Key: This DIY open source hardware connects inline with any UPDI programmer to provide a HV UPDI programming solution for tinyAVR® 0/1/2 series MCUs. Compatible with UPDI programmers that operate with jtag2updi, avrdude, pyupdi, MPLAB X IDE, MPLAB X IPE, PlatformIO and Arduino IDE using any target voltage from 3 to 5V.](https://github.com/Dlloydev/Updi-Key)
* [Create a 12V version of microUPDI · Issue #3 · MCUdude/microUPDI](https://github.com/MCUdude/microUPDI/issues/3)
* [UPDI High-Voltage programmer](https://avdweb.nl/arduino/attiny3217/hv-programming) -- Simple UPDI high voltage pulse generator

### ATtiny / AVR Dx/Ex both supported

* [\[MULTIX UPDI4AVR Programmer\] modernAVR世代専用HV対応プログラム書込器 | 朝日薫 / K.Sato](https://askn37.github.io/product/UPDI4AVR/)
* [PICerFT](http://einstlab.web.fc2.com/PICerFT/PICerFT.html)
