# Multiplox
Simplify management of multiple video game consoles, including joystick multiplexing, using Arduino based hardware nodes.

*This project has just started. Nothing is usable until this line is removed.*

## The Concept
In my mancave, I have one TV. Above, I have a shelf with 12 video game consoles. I prefer playing them all sitting in my couch about 2.5 meters from the screen. There are a few problems I need to solve, of which the greatest is the joypads. Having two joypads for each console, it sums up to 24 of them, with extension cords hanging over the screen creating a quite messy experience.

I've been thinking of different solutions to the problem, finally coming up with what I call multiplox. You connect up to four joypads (or whatever control unit you prefer) to one plox unit. For all your video game consoles, you also connect one plox unit each. Finally, you connect all your plox units in one single chain via standard CAT5 TP cables into a multiplox network. Each plox unit is configured for the respecive system, making any system compatible with any controller.

## Design Goals
* One size fits all - only one kind of plox units. System specific connection via breakout cable
* Breakout cable identify itself, letting the plox unit know details about the system
* Single power source shared between all nodes in the multiplox network
* ...

## Implementation
A plox unit consists of an Arduino and an MCP2515 CAN module as SPI slave. To the Arduino is a DB25 female breakout connector and to the MCP2515 are two RJ45 connectors for CAN BUS and power.

Using https://github.com/coryjfowler/MCP_CAN_lib for CAN communication.

## Protocol
Communication between plox units is based on CAN with standard 11 bits ID and up to 8 bytes of data.

### ID
| Bit  | Description                                           |
|------|-------------------------------------------------------|
| 0-3  | Sending unit ID. Lower has priority.                  |
| 4-7  | Receiving unit ID. Use sending unit ID for broadcast. |
| 8-10 | Message type.                                         |

| Message Type | Description       |
|-------------:|-------------------|
| 0            | Controller 0 data |
| 1            | Controller 1 data |
| 2            | Controller 2 data |
| 3            | Controller 3 data |
| 4            |                   |
| 5            |                   |
| 6            |                   |
| 7            |                   |

### Data

