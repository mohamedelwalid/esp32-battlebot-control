# BattleBot ESP32 Control System

Embedded control software and electronics for a tracked BattleBot, developed as part of **TMM4150 – Machine Design and Mechatronics** at NTNU.

The robot was built by a team of seven Mechanical Engineering students. My responsibilities were **electronics and programming**, including ESP32 implementation, controller input, motor-control logic, PCB development, testing, and system integration.

This repository covers the software and electronics scope of the project. The mechanical design, CAD, manufacturing, and assembly process are documented separately in the project case study on my portfolio website.

<p align="center">
  <img src="images/final-robot.jpg" alt="Completed BattleBot" width="600">
</p>

---

## Overview

The robot is controlled wirelessly using a DualShock 4 controller connected to an ESP32 over Bluetooth.

The ESP32 reads throttle, steering, and mode inputs and drives two brushed DC motors independently through an L298N dual H-bridge.

**Features:** differential steering, turn-in-place mode, reduced-speed crawl mode, variable PWM speed control, and battery-voltage sensing.

---

## Electronics

<p align="center">
  <img src="images/electronics-setup.jpg" alt="Integrated electronics setup" width="600">
</p>

| Component | Function |
|---|---|
| ESP32 development board | Bluetooth communication, control logic, and PWM generation |
| DualShock 4 controller | Wireless throttle, steering, and mode input |
| L298N dual H-bridge | Bidirectional control of both motors |
| Two brushed DC gear motors | Independent left and right track propulsion |
| MT3608 boost converter | Steps the battery voltage up for the motor system |
| Custom protection and sensing PCB | Fuse integration, common ground, and battery-voltage sensing |

<p align="center">
  <img src="documentation/wiring-diagram.png" alt="Wiring diagram" width="750">
</p>

Power flows from the battery through the protection and sensing PCB, into the MT3608 boost converter, and then to the L298N motor driver.

The ESP32 sends PWM and direction signals to the driver, which controls both motors independently.

### Custom PCB

<p align="center">
  <img src="documentation/bms-pcb-layout.png" alt="Protection and voltage-sensing PCB layout" width="600">
</p>

The custom PCB provides fuse integration, common ground distribution, external wiring connections, and battery-voltage sensing through a resistor divider:

```text
Vout = Vin × R2 / (R1 + R2)
```

The resistor values were selected to keep the measurement signal within the ESP32's safe ADC input range.

The current firmware reads and logs the raw ADC value. Converting this into a calibrated battery voltage would be a natural next step.

---

## Control Flow

<p align="center">
  <img src="documentation/activity-diagram.png" alt="Software activity diagram" width="750">
</p>

Each loop reads the battery input and controller state, calculates either turn-in-place or differential steering commands, applies the crawl-mode PWM limit when active, and writes the resulting outputs to both motors.

---

## Controller Mapping

| Input | Function |
|---|---|
| `R2` / `L2` | Forward / reverse throttle |
| Left stick, X-axis | Steering |
| Hold `Cross` | Turn-in-place mode |
| Hold `Circle` | Crawl mode with reduced maximum PWM output |

A dead zone around the center of the analog stick reduces unintended steering from small input variations.

---

## ESP32 Pin Configuration

| Function | GPIO |
|---|---:|
| Right motor PWM enable | 12 |
| Left motor PWM enable | 33 |
| Right motor direction, IN1 / IN2 | 14 / 27 |
| Left motor direction, IN3 / IN4 | 25 / 26 |
| Battery-voltage ADC input | 34 |

PWM configuration:

```text
Frequency: 5000 Hz
Resolution: 8-bit
Output range: 0–255
```

---

## Software Structure

- **`getSteering()`** — reads throttle and steering input, applies the dead zone, and calculates per-track speed commands.
- **`setMotor()`** — converts a signed speed command into direction signals and PWM output for the selected track.
- **`getMeasurement()`** — reads the battery-voltage ADC channel and logs the raw value over serial.

---

## Repository Structure

```text
battlebot-esp32-control/
├── software/
│   └── battlebot_controller/
│       └── battlebot_controller.ino
├── documentation/
│   ├── activity-diagram.png
│   ├── bms-pcb-layout.png
│   └── wiring-diagram.png
├── images/
│   ├── electronics-setup.jpg
│   ├── electronics-prototype.jpg
│   └── final-robot.jpg
└── README.md
```

---

## Setup

Requires the ESP32 Arduino board package and the `PS4Controller` library.

1. Open `software/battlebot_controller/battlebot_controller.ino`.
2. Replace the placeholder controller address:

```cpp
PS4.begin("XX:XX:XX:XX:XX:XX");
```

3. Select the correct ESP32 board and serial port.
4. Compile and upload the firmware.
5. Before operating the complete robot, lift the tracks or disconnect the drivetrain.
6. Confirm that both motors start at zero, then test throttle, steering, turn-in-place mode, crawl mode, and the battery reading.

---

## Testing

<p align="center">
  <img src="images/electronics-prototype.jpg" alt="Electronics prototype" width="550">
</p>

The electronics and control logic were first tested on a breadboard before integration into the completed robot.

The breadboard setup was used to test Bluetooth pairing, motor direction, PWM response, steering, and battery sensing before final integration.

The completed control system was used during robot testing and competition.

---

## Known Limitations & Next Steps

- No automatic motor stop is implemented for Bluetooth disconnection.
- Battery monitoring currently logs raw ADC values rather than calibrated voltage.
- Acceleration commands are applied directly, and the steering dead zone is fixed in the source code.
- A more efficient motor driver and a more modular firmware structure would be natural improvements.

---

## Contributions

The embedded control software was developed collaboratively by **Mohamed Elwalid Fadul** and **Malte vor dem Esche** as part of a seven-person student team.
