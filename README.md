# BattleBot ESP32 Control System

Embedded control software and electronics implementation for a tracked BattleBot developed as part of **TMM4150 – Machine Design and Mechatronics** at NTNU.

The complete robot was developed by a team of seven Mechanical Engineering students. My main responsibilities were within **electronics and programming**, including ESP32 implementation, controller input, motor-control logic, testing, and system integration.

This repository focuses on the software and electronics. The mechanical design, CAD development, manufacturing, and assembly process are presented separately in the project case study on my portfolio website.

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
- Battery-voltage measurement

---

## Electronics

![Electronics setup](images/electronics-setup.jpg)

The main electronic components used in the control system were:

| Component | Function |
|---|---|
| ESP32 development board | Main controller, Bluetooth communication, input processing, and PWM generation |
| Sony DualShock 4 controller | Wireless throttle, steering, and mode input |
| L298N dual H-bridge | Bidirectional control of the two DC motors |
| Two brushed DC gear motors | Independent left and right track propulsion |
| MT3608 boost converter | Increases the battery voltage supplied to the motor system |
| Battery-management circuit | Battery protection and system isolation |
| Voltage divider | Reduces the measured battery voltage to a safe ESP32 ADC level |
| Fuse | Overcurrent protection |

The ESP32 was selected because it provides built-in Bluetooth communication and sufficient processing capability for controller input, steering calculations, and PWM motor control.

---

## System Architecture

```text
DualShock 4 Controller
          |
          | Bluetooth
          v
        ESP32
          |
          | PWM and direction signals
          v
    L298N Motor Driver
       |           |
       v           v
 Left Motor    Right Motor
```

The ESP32 acts as the main controller in the system.

It receives wireless input from the DualShock 4 controller, calculates the required output for each track, and sends direction and PWM signals to the motor driver.

The motor driver then controls the direction and speed of the left and right motors independently.

---

## Activity Diagram

![Control software activity diagram](images/activity-diagram.png)

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

A simplified representation of the program flow is shown below:

```text
Start
  |
  v
Initialize pins, PWM and Bluetooth
  |
  v
Set both motors to zero
  |
  v
Read controller input and battery signal
  |
  v
Is Cross pressed?
  |                     |
 Yes                    No
  |                     |
  v                     v
Calculate opposing     Read throttle
track speeds           and steering
  |                     |
  |                     v
  |                Is Circle pressed?
  |                     |
  |               Yes         No
  |                |           |
  |                v           |
  |          Limit PWM output  |
  |                |           |
  +----------------+-----------+
                   |
                   v
          Send commands to motors
                   |
                   v
                Repeat
```

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

Holding the `Cross` button activates turn-in-place mode.

In this mode, the horizontal position of the left analog stick controls both tracks in opposite directions.

This allows the robot to rotate around its approximate center without forward or backward movement.

### Crawl Mode

Holding the `Circle` button activates crawl mode.

The calculated motor commands are mapped to a reduced PWM range, limiting the maximum speed of the robot.

This mode was implemented to provide more precise control during positioning and close-range movement.

---

## Differential Steering

The robot uses two independently driven tracks.

During straight-line movement, both motors receive approximately the same command:

```text
Left motor  = throttle
Right motor = throttle
```

When steering to the right, the right track is slowed down while the left track maintains the requested throttle:

```text
Left motor  = throttle
Right motor = throttle - steering correction
```

When steering to the left, the left track is slowed down:

```text
Right motor = throttle
Left motor  = throttle - steering correction
```

A small dead zone is applied around the center of the analog stick to reduce unintended steering from small input variations.

---

## Motor Control

The `setMotor()` function receives:

- A signed motor-speed command
- A selection of the left or right track

The sign of the command determines direction:

```text
Positive value  -> forward
Negative value  -> reverse
Zero            -> stop
```

The magnitude determines the PWM duty cycle.

The motor command is constrained to the valid 8-bit PWM range:

```text
-255 to 255
```

For reverse movement, the direction pins are switched and the absolute speed value is sent to the PWM channel.

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

This function reads the throttle and steering inputs from the controller.

It:

- Reads `R2` for forward throttle
- Reads `L2` for reverse throttle
- Reads the horizontal value of the left analog stick
- Applies a steering dead zone
- Calculates separate speed commands for the left and right tracks

### `setMotor()`

This function converts a signed speed command into motor-driver output.

It:

- Constrains the requested speed
- Selects the left or right motor
- Sets the direction pins
- Writes the PWM duty cycle
- Stops the motor when the requested speed is zero

### `getMeasurement()`

This function reads the battery-voltage signal through the ESP32 ADC input.

The voltage is first reduced using a resistor voltage divider so that the signal remains within the safe input range of the ESP32.

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
│   ├── wiring-diagram.png
│   └── system-block-diagram.png
├── images/
│   ├── electronics-setup.jpg
│   └── final-robot.jpg
├── README.md
└── LICENSE
```

Suggested use of the folders:

- `software/` contains the ESP32 source code.
- `documentation/` contains technical diagrams.
- `images/` contains photographs of the electronics and final robot.
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

Enter the Bluetooth address used by the controller connection.

Use a placeholder in the public repository rather than publishing the address directly:

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

## Testing and Development

The electronics and software were initially tested using a breadboard setup before being integrated into the complete robot.

![Electronics prototype](images/electronics-prototype.jpg)

The initial setup included:

- ESP32 development board
- L298N motor driver
- Two brushed DC motors
- Voltage converter
- Battery connection
- DualShock 4 controller

Testing focused on:

- Bluetooth connection
- Controller input
- Forward and reverse operation
- Left and right motor direction
- Differential steering
- Turn-in-place mode
- Crawl mode
- PWM response
- Battery-voltage reading
- Full-system integration

Several code adjustments were made during testing to improve steering response and low-speed control.

---

## Results

The final implementation provided wireless control of both tracks using the DualShock 4 controller.

The following functions were successfully integrated:

- Forward and reverse driving
- Proportional throttle input
- Differential steering
- Independent motor direction control
- Turn-in-place mode
- Reduced-speed crawl mode
- Battery-voltage ADC reading

The electronics and software were integrated into the final robot and used during testing and competition.

![Final robot](images/final-robot.jpg)

---

## Limitations

This was an educational team project completed within the available course period and with the components provided or available in the laboratory.

The implementation has several limitations:

- The motors are not automatically stopped based on a dedicated controller-disconnection routine.
- The battery-monitoring function currently outputs a raw ADC value rather than a fully calibrated voltage.
- The acceleration command is applied directly without ramping.
- The controller dead zone is fixed in the source code.
- The complete program is stored in one main source file.
- The L298N motor driver has relatively high electrical losses compared with newer motor drivers.
- The steering system was tuned through practical testing rather than a formal control-system model.

These limitations reflect the scope and time constraints of the project.

---

## Possible Improvements

Reasonable future improvements could include:

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

This repository contains team project material and is presented as documentation of the software and electronics work completed during the course.

---

## Related Case Study

A separate portfolio case study presents the complete engineering project, including:

- Project requirements
- Concept development
- Mechanical design
- CAD
- Manufacturing
- Assembly
- Electronics integration
- Testing
- Final results

Portfolio case study:

```text
Coming soon
```
