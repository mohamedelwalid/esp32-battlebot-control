# BattleBot ESP32 Control System

Embedded control software and electronics implementation for a tracked BattleBot developed as part of **TMM4150 – Machine Design and Mechatronics** at NTNU.

The complete robot was developed by a team of seven Mechanical Engineering students. My main responsibilities were within **electronics and programming**, including ESP32 implementation, controller input, motor-control logic, testing, and system integration.

This repository focuses on the software and electronics portion of the project. The mechanical design, CAD development, manufacturing, and assembly process are presented separately in the project case study on my portfolio website.

<p align="center">
  <img src="images/final-robot.jpg" alt="Completed BattleBot" width="600">
  <br>
  <em>Completed tracked BattleBot developed for TMM4150 at NTNU.</em>
</p>

---

## Project Overview

The robot is controlled wirelessly using a Sony DualShock 4 controller connected to an ESP32 through Bluetooth.

The ESP32 reads throttle, steering, and button inputs from the controller and converts them into speed and direction commands for two brushed DC motors. The motors are controlled independently through an L298N dual H-bridge motor driver.

The implemented control system supports:

- Forward and reverse driving
- Differential steering
- Variable motor speed using PWM
- Turn-in-place control
- Reduced-speed crawl mode
- Battery-voltage sensing through an ESP32 ADC input

---

## Electronics

<p align="center">
  <img src="images/electronics-setup.jpg" alt="Integrated electronics setup" width="600">
  <br>
  <em>Integrated electronics setup containing the ESP32, L298N motor driver, voltage converter, protection circuit, and power connections.</em>
</p>

| Component | Function |
|---|---|
| ESP32 development board | Main controller, Bluetooth communication, input processing, and PWM generation |
| Sony DualShock 4 controller | Wireless throttle, steering, and mode input |
| L298N dual H-bridge | Bidirectional control of the two DC motors |
| Two brushed DC gear motors | Independent left and right track propulsion |
| MT3608 boost converter | Increases the battery voltage supplied to the motor system |
| Protection and sensing PCB | Fuse integration, power connections, common ground, and battery-voltage sensing |
| Voltage divider | Reduces the measured battery voltage to a safe ESP32 ADC level |
| Fuse | Provides overcurrent protection and allows the power circuit to be isolated |

The ESP32 was selected because it provides built-in Bluetooth communication and sufficient processing capability for controller input, steering calculations, and PWM motor control.

---

## Wiring

<p align="center">
  <img src="documentation/wiring-diagram.png" alt="Wiring diagram" width="850">
  <br>
  <em>Simplified wiring diagram showing the connections between the battery, protection circuit, voltage converter, ESP32, motor driver, and motors.</em>
</p>

The diagram shows how the main electrical components were connected during the project.

The battery supplies the protection and sensing circuit. The voltage is then increased by the MT3608 boost converter before being supplied to the motor driver. The ESP32 sends direction and PWM signals to the L298N, while the two motors are controlled independently through the driver outputs.

---

## Protection and Voltage-Sensing PCB

<p align="center">
  <img src="documentation/bms-pcb-layout.png" alt="Protection and voltage-sensing PCB layout" width="700">
  <br>
  <em>PCB layout used for fuse integration, common ground distribution, external connections, and battery-voltage sensing.</em>
</p>

The project included a custom PCB used as part of the battery protection and monitoring system.

The board was designed to provide:

- A connection point between the battery and the rest of the electrical system
- Fuse integration for overcurrent protection
- A shared ground connection
- Screw-terminal connections for external wiring
- A resistor voltage divider for battery-voltage sensing

The voltage-divider output was connected to an ESP32 ADC input. Because the battery voltage was higher than the permitted ESP32 input voltage, the resistor network reduced the signal before it reached the microcontroller.

The voltage-divider relationship is:

```text
Vout = Vin × R2 / (R1 + R2)
```

The resistor values were selected so that the expected maximum battery voltage produced an ESP32-safe measurement signal.

The current software implementation reads and prints the raw ADC value. Conversion to a calibrated battery voltage was identified as a possible future improvement.

---

## Control Flow

<p align="center">
  <img src="documentation/activity-diagram.png" alt="Software activity diagram" width="850">
  <br>
  <em>Activity diagram showing controller input handling, driving-mode selection, steering calculations, PWM limiting, and motor output.</em>
</p>

The main software loop follows this sequence:

1. Read the battery-voltage input.
2. Read the current controller state.
3. Check whether turn-in-place mode is active.
4. If turn-in-place mode is active, calculate opposite motor commands.
5. Otherwise, calculate normal differential steering.
6. Check whether crawl mode is active.
7. Limit the PWM output when crawl mode is enabled.
8. Send the calculated direction and PWM values to both motors.
9. Repeat the loop.

---

## Controller Mapping

### Normal Driving

| Input | Function |
|---|---|
| `R2` | Forward throttle |
| `L2` | Reverse throttle |
| Left analog stick, horizontal axis | Steering |

The trigger values determine the requested throttle.

The horizontal position of the left analog stick determines how much the speed of one track should be reduced relative to the other.

### Turn-in-Place Mode

Holding `Cross` activates turn-in-place mode.

The horizontal position of the left analog stick controls both tracks in opposite directions, allowing the robot to rotate around its approximate center without forward or backward movement.

### Crawl Mode

Holding `Circle` activates crawl mode.

The calculated motor commands are mapped to a reduced PWM range, limiting the maximum speed for more precise control during positioning and close-range movement.

---

## Differential Steering

During straight-line movement, both motors receive approximately the same command:

```text
Left motor  = throttle
Right motor = throttle
```

When steering right, the right track is slowed while the left track maintains the requested throttle:

```text
Left motor  = throttle
Right motor = throttle - steering correction
```

When steering left, the left track is slowed while the right track maintains the requested throttle:

```text
Right motor = throttle
Left motor  = throttle - steering correction
```

A small dead zone is applied around the center of the analog stick to reduce unintended steering caused by small input variations.

---

## Motor Control

The `setMotor()` function receives:

- A signed motor-speed command
- A selection of the left or right track

The sign of the command determines the motor direction:

```text
Positive value  -> forward
Negative value  -> reverse
Zero            -> stop
```

The magnitude determines the PWM duty cycle.

The command is constrained to the valid 8-bit PWM range:

```text
-255 to 255
```

For reverse movement, the direction pins are switched and the absolute value of the speed command is sent to the PWM channel.

---

## ESP32 Pin Configuration

| Function | ESP32 GPIO |
|---|---:|
| Right motor PWM enable | 12 |
| Left motor PWM enable | 33 |
| Right motor direction input 1 | 14 |
| Right motor direction input 2 | 27 |
| Left motor direction input 1 | 25 |
| Left motor direction input 2 | 26 |
| Battery-voltage ADC input | 34 |

PWM configuration:

```text
Frequency: 5000 Hz
Resolution: 8-bit
Output range: 0–255
```

The pin assignments are defined near the beginning of the source file and can be changed for a different wiring configuration.

---

## Software Structure

The program is organized around three main functions.

### `getSteering()`

Reads throttle and steering input from the controller.

The function:

- Reads `R2` for forward throttle
- Reads `L2` for reverse throttle
- Reads the horizontal value of the left analog stick
- Applies a steering dead zone
- Calculates separate speed commands for the left and right tracks

### `setMotor()`

Converts a signed speed command into motor-driver output.

The function:

- Constrains the requested speed
- Selects the left or right motor
- Sets the direction pins
- Writes the PWM duty cycle
- Stops the motor when the requested speed is zero

### `getMeasurement()`

Reads the battery-voltage signal through the ESP32 ADC input.

The voltage is reduced using the resistor voltage divider on the protection and sensing PCB before reaching the ESP32.

The current implementation periodically prints the raw ADC value through the serial connection.

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
├── README.md
└── LICENSE
```

- `software/` contains the ESP32 source code.
- `documentation/` contains the activity diagram, PCB layout, and wiring diagram.
- `images/` contains photographs of the electronics prototype, integrated electronics, and completed robot.
- `README.md` provides an overview of the implementation.

---

## Dependencies

The software was developed using the Arduino framework for ESP32.

Required library:

- `PS4Controller`

The library is used to connect the DualShock 4 controller to the ESP32 and read buttons, triggers, and analog-stick inputs.

---

## Setup

### 1. Install ESP32 Support

Install the ESP32 board package in the Arduino IDE.

### 2. Install the Controller Library

Install the required `PS4Controller` library.

### 3. Open the Source Code

Open:

```text
software/battlebot_controller/battlebot_controller.ino
```

### 4. Configure the Controller Address

Replace the placeholder with the Bluetooth address used for the controller:

```cpp
PS4.begin("XX:XX:XX:XX:XX:XX");
```

### 5. Select the Board

Select the correct ESP32 development board and serial port in the Arduino IDE.

### 6. Upload the Program

Compile and upload the program to the ESP32.

### 7. Test the System

Before operating the complete robot:

1. Lift the tracks from the ground or disconnect the drivetrain.
2. Confirm that both motors are stopped during startup.
3. Test forward and reverse movement.
4. Check that both motors rotate in the correct direction.
5. Test left and right steering.
6. Test turn-in-place mode.
7. Test crawl mode.
8. Check the battery-voltage ADC reading.

---

## Testing and Results

The electronics and software were initially tested using a breadboard setup before being integrated into the complete robot.

<p align="center">
  <img src="images/electronics-prototype.jpg" alt="Electronics prototype" width="600">
  <br>
  <em>Breadboard prototype used to test Bluetooth communication, motor direction, PWM control, steering, and battery-voltage sensing.</em>
</p>

Testing covered:

- Bluetooth connection
- Controller input
- Forward and reverse operation
- Left and right motor direction
- Differential steering
- Turn-in-place mode
- Crawl mode
- PWM response
- Battery-voltage sensing
- Full-system integration

Several code adjustments were made during testing to improve steering response and low-speed control.

The final implementation provided wireless control of both tracks and included the planned driving modes. The control system was integrated into the completed robot and used during testing and competition.

---

## Limitations

This was an educational team project completed within the available course period and with the components available in the laboratory.

The implementation has several limitations:

- The motors are not automatically stopped through a dedicated controller-disconnection routine.
- Battery monitoring currently outputs a raw ADC value rather than a calibrated voltage.
- Acceleration is applied directly without ramping.
- The controller dead zone is fixed in the source code.
- The complete program is stored in one main source file.
- The L298N motor driver has relatively high electrical losses compared with newer motor drivers.
- Steering was tuned through practical testing rather than a formal control-system model.

---

## Possible Improvements

Reasonable future improvements include:

- Automatically stopping both motors if the Bluetooth connection is lost
- Converting the ADC reading into a calibrated battery voltage
- Adding a low-battery warning
- Adding acceleration and deceleration ramping
- Moving pin assignments and settings into a separate configuration file
- Separating motor control and controller handling into individual source files
- Making the steering dead zone configurable
- Replacing the L298N with a more efficient motor driver
- Adding current or motor-temperature monitoring
- Testing the steering calculations independently from the hardware

---

## Project Context

This repository documents the software and electronics portion of a complete BattleBot project developed in:

**TMM4150 – Machine Design and Mechatronics**  
**Norwegian University of Science and Technology – NTNU**

The complete project also included:

- Mechanical design
- CAD modelling
- Material selection
- Manufacturing
- Assembly
- Testing
- Competition participation

These parts are presented separately in the portfolio case study.

---

## Team and Contribution

The robot was developed by a team of seven Mechanical Engineering students at NTNU.

The work was divided between mechanical design, manufacturing, electronics, and programming.

My main responsibilities were within:

- Electronics
- ESP32 programming
- Controller implementation
- Motor-control logic
- Testing
- System integration

This repository contains team project material and documents the software and electronics work completed during the course.
