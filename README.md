# ReTurn

## Introduction

ReTurn is an interactive corridor powered by many connected microcontrollers.
In this corridor, there are modular units, that can be hotplugged while the system is running.

## System Setup

Each unit has one board to control all peripherals of that unit.
This board can be any 5V microcontroller with enough ports to power all peripherals.
In our setup, custom Arduino Mega 2560 boards were used.
The table below gives an overview of all peripherals used by the system.

| Port | Hardware | Function |
| ------------- | ------------- | ---------- |
| A4 | Servo Motor | Panel Movement |
| A6 | PWM MosFET | Light Control |
| UART2 | Serial to Previous Unit | Inter-Module Communication |
| UART3 | Serial to Next Unit | Inter-Module Communication |
| I2C | Time of Flight Sensor (VL53L0X) | Human Detection |
| I2C | Control Panel Microcontroller | Configuration Tuning |
| A2, A3 | Chainable LED | Debugging |

## Code Layout

The code is divided in two distinct layers.
One layer runs in a continuous loop, handling each event as fast as possible.
This layer contains the human detection, communication and the control panel.
The second layer runs at a fixed 50 Hz framerate, to make it easier to program for.
This layer contains the presence grouping, motor control, light dimming and wave physics.

### Decentralization

There is no central server controlling the setup.
Each microcontroller tracks all events, and decides what to do by using that data.
The microcontroller communicate events they perceive over chained serial to all other units.
Because of this setup, changing constants in the system is a tedious process.
To prevent having to re-upload the code to all units for every small change, a control panel can be used.
There is a global configuration in each unit that contains all numerical constants.
The control panel used for the demo system in the exhibition was an Arduino Nano 33 IoT,
running the NanoSerialTest found in the "test" folder.
However, any Arduino can implement the control panel protocol over I2C.
When the control panel uploads a command to the unit it is connected to,
that unit will distribute it over serial to all other units.
Each unit will update their global configuration during runtime, making it easier to test changes.

### Physics

The goal of the corridor is to show waves flowing through the panels.
This is done by emulating the physics of real waves, with an amplitude, size and velocity.
Every frame (20 ms) the velocity is updated by estimating where people are,
and then the velocity is used to update the position.
When two waves collide, they are merged into one.
Similarly, if humans spread out into a larger area, the wave is split.

### Setup

To compile the main code, a modified Time of Flight library is needed. This can be found here: https://github.com/amirzaidi/Grove-Ranging-sensor-VL53L0X
