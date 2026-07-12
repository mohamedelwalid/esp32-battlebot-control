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
 
The ESP32 receives wireless input from the DualShock 4 controller, calculates the required output for each track, and sends direction and PWM signals to the motor driver, which controls the direction and speed of the left and right motors independently.
 
---
 
## Control Flow
 
![Control software activity diagram](images/activity-diagram.png)
 
The main software loop follows this sequence:
 
1. Read the battery-voltage input.
2. Read the current controller state.
3. If turn-in-place mode (`Cross` held) is active, calculate opposite motor commands; otherwise calculate normal differential steering.
4. If crawl mode (`Circle` held) is active, limit the PWM output.
5. Send the calculated direction and PWM values to both motors.
6. Repeat.
---
 
## Controller Mapping
 
### Normal Driving
 
| Input | Function |
|---|---|
| `R2` | Forward throttle |
| `L2` | Reverse throttle |
| Left analog stick, horizontal axis | Steering |
 
The trigger values determine the requested throttle. The horizontal position of the left analog stick determines how much the speed of one track should be reduced relative to the other.
 
### Turn-in-Place Mode
 
Holding `Cross` activates turn-in-place mode. The horizontal position of the left analog stick controls both tracks in opposite directions, allowing the robot to rotate around its approximate center without forward or backward movement.
 
### Crawl Mode
 
Holding `Circle` activates crawl mode. The calculated motor commands are mapped to a reduced PWM range, limiting the maximum speed for more precise control during positioning and close-range movement.
 
---
 
## Differential Steering
 
During straight-line movement, both motors receive approximately the same command:
 
```text
Left motor  = throttle
Right motor = throttle
```
 
When steering right, the right track is slowed while the left track maintains the requested throttle (and vice versa when steering left):
 
```text
Right turn: Left motor = throttle | Right motor = throttle - steering correction
Left turn:  Right motor = throttle | Left motor  = throttle - steering correction
```
 
A small dead zone is applied around the center of the analog stick to reduce unintended steering from small input variations.
 
---
 
## Motor Control
 
The `setMotor()` function receives a signed motor-speed command and a selection of the left or right track.
 
```text
Positive value  -> forward
Negative value  -> reverse
Zero            -> stop
```
 
The magnitude determines the PWM duty cycle. The command is constrained to the valid 8-bit PWM range (-255 to 255). For reverse movement, the direction pins are switched and the absolute speed value is sent to the PWM channel.
 
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
 
PWM configuration: 5000 Hz, 8-bit resolution, 0–255 output range.
 
Pin assignments are defined near the beginning of the source file and can be changed for a different wiring configuration.
 
---
 
## Software Structure
 
The program is organized around three main functions.
 
**`getSteering()`** — reads `R2`/`L2` for throttle, reads the left stick's horizontal axis, applies a steering dead zone, and calculates separate speed commands for the left and right tracks.
 
**`setMotor()`** — converts a signed speed command into motor-driver output: constrains the value, selects the motor, sets direction pins, writes the PWM duty cycle, and stops the motor at zero.
 
**`getMeasurement()`** — reads the battery-voltage signal through the ESP32 ADC input, reduced beforehand by a resistor voltage divider. The current implementation periodically prints the raw ADC value over serial.
 
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
 
---
 
## Dependencies
 
Developed using the Arduino framework for ESP32.
 
Required library: `PS4Controller`
 
---
 
## Setup
 
1. Install the ESP32 board package in the Arduino IDE.
2. Install the `PS4Controller` library.
3. Open `software/battlebot_controller/battlebot_controller.ino`.
4. Set the controller's Bluetooth address — **use a placeholder in the public repo, not a real address**:
```cpp
   PS4.begin("XX:XX:XX:XX:XX:XX");
```
5. Select the correct ESP32 board and serial port.
6. Compile and upload.
7. Before operating the complete robot:
   - Lift the tracks from the ground or disconnect the drivetrain.
   - Confirm both motors are stopped at startup.
   - Test forward/reverse, left/right steering, turn-in-place, and crawl mode.
   - Check the battery-voltage ADC reading.
---
 
## Testing and Results
 
The electronics and software were first tested on a breadboard (ESP32, L298N, two motors, voltage converter, battery, DualShock 4) before integration into the complete robot.
 
![Electronics prototype](images/electronics-prototype.jpg)
 
Testing covered Bluetooth connection, controller input, forward/reverse operation, motor direction, differential steering, turn-in-place mode, crawl mode, PWM response, battery-voltage reading, and full-system integration. Several code adjustments were made during testing to improve steering response and low-speed control.
 
The final implementation provided wireless control of both tracks, with all planned functions successfully integrated and used during testing and competition.
 
![Final robot](images/final-robot.jpg)
 
---
 
## Limitations
 
- Motors are not automatically stopped based on a dedicated controller-disconnection routine.
- Battery monitoring currently outputs a raw ADC value rather than a calibrated voltage.
- Acceleration is applied directly without ramping.
- The controller dead zone is fixed in source code.
- The complete program lives in one main source file.
- The L298N motor driver has relatively high electrical losses compared with newer drivers.
- Steering was tuned through practical testing rather than a formal control-system model.
## Possible Improvements
 
- Automatically stop both motors if the Bluetooth connection is lost.
- Convert the ADC reading into a calibrated battery voltage, with a low-battery warning.
- Add acceleration/deceleration ramping.
- Move pin assignments and settings into a separate configuration file.
- Separate motor control and controller handling into individual source files.
- Make the steering dead zone configurable.
- Replace the L298N with a more efficient motor driver.
- Add current or motor-temperature monitoring.
- Test the steering calculations independently from the hardware.
---
 
## Project Context
 
Developed for **TMM4150 – Machine Design and Mechatronics**, Norwegian University of Science and Technology (NTNU). The complete project also included mechanical design, CAD modelling, material selection, manufacturing, assembly, testing, and competition participation — presented separately in the portfolio case study.
 
## Team and Contribution
 
Developed by a team of seven Mechanical Engineering students at NTNU, split across mechanical design, manufacturing, electronics, and programming. My main responsibilities: electronics, ESP32 programming, controller implementation, motor-control logic, testing, and system integration.
 
This repository documents the software and electronics portion of the project.
