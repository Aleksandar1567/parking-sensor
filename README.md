# Parking Sensor — MSP430

An ultrasonic parking sensor built around the **MSP430** microcontroller and the **HC-SR04** distance sensor. Measures distance to nearby objects and provides feedback through a PWM-driven LED and UART serial output. Designed with low power consumption in mind using LPM3 sleep mode between measurements.

---

## How It Works

1. The MCU sends a ~10 µs trigger pulse to the HC-SR04 via **P6.3**.
2. The sensor responds with an echo pulse on **P2.0**, whose width is proportional to the measured distance.
3. Timer **TA1** captures both edges of the echo pulse in capture mode.
4. Distance is calculated from the pulse width and the speed of sound (~340 m/s).
5. The result is transmitted over **UART** and used to set the blink frequency of a PWM LED — the closer the object, the faster the blink.
6. Below the `DISTANCE_THRESHOLD` (10 cm), the LED turns on solid.
7. Between measurements, the MCU enters **LPM3** to minimize power consumption.

---

## Hardware

| Component | Description |
|---|---|
| MCU | MSP430 (SMCLK @ ~1.048 MHz, ACLK @ 32.768 kHz) |
| Sensor | HC-SR04 ultrasonic distance sensor |
| Trigger pin | P6.3 |
| Echo pin | P2.0 (TA1.CCI1A capture input) |
| LED (solid) | P1.0 |
| LED (PWM) | P1.2 (TA0.1), P1.3 (TA0.2) |
| UART TX | P4.4 / P4.5 — USCI A1, 9600 bps |

---

## Configuration

Defined in `main.c`:

```c
#define DISTANCE_THRESHOLD  10      // cm — solid LED on below this
#define MEASURE_INTERVAL    2048    // timer ticks (~250 ms between measurements)
```

---

## Distance Calculation

The echo pulse is captured using **Timer TA1** running at ACLK/4 = 8192 Hz.

```
distance (cm) = pulse_ticks × 34000 / 16384
```

The result is an 8-bit value (0–255 cm) sent over UART on each measurement cycle.

---

## LED Feedback

- **PWM blink rate** (P1.2 / P1.3): period scales with distance — closer = faster blink.
  - Period formula: `period = 327 × distance` (maps ~200 cm to the 16-bit timer maximum).
- **Solid LED** (P1.0): turns on when `distance ≤ DISTANCE_THRESHOLD`.

---

## UART Output

- Interface: USCI A1 (`UCA1`)
- Baud rate: **9600 bps**
- Clock: SMCLK (~1.048 MHz), BRW = 109, BRS = 2
- Each measurement cycle transmits a single byte — the distance in centimetres.

---

## Interrupt Service Routines

The ISRs for **TIMER1_A0** (measure interval tick) and **TIMER1_A1** (echo edge capture) are implemented in **assembler** (the C stubs are commented out in `main.c` for reference).

Both ISRs exit LPM3 on return (`__low_power_mode_off_on_exit()`), waking the main loop to process each event.

---

## Build

The project targets the MSP430 toolchain. Open in **Code Composer Studio** or build with `msp430-gcc`:

```bash
msp430-gcc -mmcu=<your_device> -o parking_sensor.elf main.c
```

Replace `<your_device>` with your specific MSP430 variant (e.g. `msp430f5529`).

---

## Author

**Aleksandar Petos** — June 2023
