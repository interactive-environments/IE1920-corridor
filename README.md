# Back2Earth

## Introduction

Back2Earth is an interactive corridor powered by many connected microcontrollers.
In this corridor, there are modular units, that can be hotplugged while the system is running.

## System Setup

Each unit has one board to control all peripherals of that unit.
This board can be any 5V microcontroller with enough ports to power all peripherals.
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

Track Everything
Serial
Control Panel

### Physics

Presence
Wave
