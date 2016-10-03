# Multiplox

Simplify management of multiple video game consoles, including joystick multiplexing, using Arduino based hardware nodes.

*This project has just started. Nothing is usable until this line is removed.*

## The Concept
In my mancave, I have one TV. Above, I have a shelf with 12 video game consoles. I prefer playing them all sitting in my couch about 2.5 meters from the screen. There are a few problems I need to solve, of which the greatest is the joypads. Having two joypads for each console, it sums up to 24 of them, with extension cords hanging over the screen creating a quite messy experience.

I've been thinking of different solutions to the problem, finally coming up with what I call multiplox. You connect up to four joypads (or whatever control unit you prefer) to one plox unit. For all your video game consoles, you also connect one plox unit each. Finally, you connect all your plox units in one single chain via standard CAT5 TP cables into a multiplox network. Each plox unit is configured for the respecive system, making any system compatible with any controller.

## Design Goals
* One size fits all - only one kind of plox units. System specific connection via breakout cable
* (Breakout cable identify itself, letting the plox unit know details about the system ... /To be reconsidered/)
* (Single power source shared between all nodes in the multiplox network ... /To be reconsidered/)
* ...

## Implementation
A plox unit consists of an Arduino and an MCP2515 CAN module as SPI slave. To the Arduino is a DB25 female breakout connector and to the MCP2515 are two RJ45 connectors for CAN BUS and power.

Using https://github.com/coryjfowler/MCP_CAN_lib for CAN communication.

## Protocol
Communication between plox units is based on CAN with standard 11 bits ID and up to 8 bytes of data.

### ID
| Bit  | Description                                           |
|------|-------------------------------------------------------|
| 0-7  | Receiving unit ID. Lower has priority.                  |
| 8    | Message Type: 0 = _Meta_, 1 = _Action_                |
| 9-10 | Controller number 0, 1, 2 or 3.                       |

The Message Type can be either _Meta_ or _Action_. _Meta_ is used for handshaking, configuration, status messages etc, while _Action_ is the actual controller data.

_Buttons_ has a defined set of digital and analog buttons in a fixed order. Any base unit implementing the _ protocol can be fully or partly controlled by any standard controller. Not all controllers have all buttons included in the standard set. In those cases, the present subset will be implemented while the missing buttons will be unavailable. 

Contrary, some controllers have other buttons or features not defined in the standard protocol. If there is an intersection between the standard definition and the present buttons, the controller can still send a standard message. All other signals need to be sent with the special protocol type. This means 

### Handshaking
In order to receive _Action_ messages, a base plox must have an active subscription with one or more controller plox units. The subscription must be renewed every four seconds and will timeout after six seconds. Issuing subscriptions is the responsibility of controller plox units. A base plox must listen to either global _Meta_ messages if no subscription is active, or subscribed _Action_ messages. This is an exclusive or. It must never listen to global and subscribed messages at the same time.

### Data

### Communication Diagram

### Instant Connection

Four wires can be configured for direct communication between a controller to a base unit. Useful for low latency applications such as light guns. There are a few caveats:
* To use the Instant Connection, two units must agree to do so. All other units must ignore the signals. 
* After agreement to use a wire, this wire is globally locked until agreement is cancelled. 
* Protocol translation in software is not possible. Hence, there can be no universal lightgun.

Agreement to set up an Instant Connection is done via a CAN _Meta_ message. The plox unit with the controller initiates the connection and the currently active base unit plox must agree or deny.

#### Canelling Agreement
The Instant Connection agreement has a fairly short life time and must be actively reissued, otherwise cancelled. The timeout is yet to be decided, but a renegociation should be held approximately every four seconds and the agreement cancelled after six seconds.

## Configuration
A Multiplox network consists of at least two and at most 16 units. Each unit needs a unique ID between 0 to 15. To setup the unit IDs, you 

1) connect all units to the CAN buss
2) press and hold the plox button for 10 seconds, until the led is starting to flash
3) press the plox button of the next unit, which will start flashing
4) if there is another plox unit, repeat from 3)
5) finally, press the plox button of the first unit again to confirm the configuration (all units should stop flasing).

The ID will be saved into the EEPROM of all plox units you have included in the configuration. Please note that any plox units you fail to configure will keep its' old ID, which might lead to ID conflicts in the network. This will not be detected as an error by the system, but there will be undefined behavior in the network.

Please reconfigure the network every time you make a change, or if you experience any strange behavior. 

## Mapping
Describing the mapping of controllers to base units. First a table of short names.

| Name     | Description                                      |
|----------|--------------------------------------------------|
| urdl     | D-pad: up, right, down, left                     |
| b0-b7    | Generic button names                             |
| a-z 0-9  | Button names as labeled on controllers           |
| st sl    | Start and select                                 |
| :x       | Autofire of x, where x can be any of above names |
| :< :>    | Slower or faster autofire of depressed button    |
| !x       | Plox button plus x on controller                 |

### Controller
| System\Bit    |  0-3 | 4   | 5   | 6   | 7   | 8   | 9   | a   | b   | c   | d   | e   | f   |
|---------------|------|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|
| Plox Standard | urdl | st  | se  | b0  | b1  | b2  | b3  | b4  | b5  | b6  | b7  | tl  | tr  |
| Atari         | <>   |     |     | 1   |     |     |     |     |     |     |     |     |     |
| CD32          | <>   | ply | !pl | red | grn | blu | yel |     |     |     |     | <>  | <>  |
| NES           | <>   | <>  | <>  | b   | a   |     |     |     |     |     |     |     |     |
| SNES          | <>   | <>  | <>  | y   | b   | x   | a   |     |     |     |     | <>  | <>  |
| AES           | <>   | <>  | <>  | a   | b   | c   | d   |     |     |     |     |     |     |
| N64           | <>   | <>  | lt  | b   | a   | y   | x   |     |     |     |     | <>  | <>  |
| GC            | <>   | <>  | lt  | b   | a   | y   | x   |     |     |     |     | <>  | <>  |
| SMS           | <>   | !1  | !2  | 1   | 2   |     |     |     |     |     |     |     |     |
| SMD           | <>   | <>  | c   | a   | b   | x   | y   | z   |     |     |     |     |     |
| Saturn        | <>   | ?   | ?   | ?   | ?   | ?   | ?   | ?   | ?   | ?   | ?   | ?   | ?   |
| Keyboard      | <>   | f1  | f2  | z   | x   | c   | v   | a   | s   | d   | f   | q   | e   |
| Mouse         | <>   | !lm | !rm | lmb | rmb | mmb |     |     |     |     |     |     |     |

| System\Bit    | 10-17 | 18-1f | 20-27 | 28-2f | 30-33 | 34-37 | 38-3b | 3c-3f |
|---------------|-------|-------|-------|-------|-------|-------|-------|-------|
| Plox Standard | a0x   | a0y   | a1x   | a1y   | a0tl  | a0tr  | a1tl  | a1tr  |
| N64           | <>    | <>    | <>    | <>    |       |       |       |       |
| GC            | <>    | <>    | <>    | <>    |       |       |       |       |
| Keyboard      | kc0   | kc1   | kc2   | kc3   |       |       |       |       |
| Mouse         | xspd  | yspd  |       |       |       |       |       |       |

Please note that for analog directions, values are signed int where up/right are represented by positive values and down/left by negative values. The value 0 means no movement.

### Base Unit
| System\Bit    |  0-3 | 4   | 5   | 6   | 7   | 8   | 9   | a   | b   | c   | d   | e   | f   |
|---------------|------|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|
| Plox Standard | urdl | st  | se  | b0  | b1  | b2  | b3  | b4  | b5  | b6  | b7  | tl  | tr  |
| Atari         | <>   |     |     | 1   |     |     |     |     |     |     |     |     |     |
| CD32          | <>   | ply |     | red | grn | blu | yel |     |     |     |     | <>  | <>  |
| NES           | <>   | <>  | <>  | b   | a   |     |     |     |     |     |     |     |     |
| SNES          | <>   | <>  | <>  | y   | b   | x   | a   |     |     |     |     | <>  | <>  |
| N64           | <>   | <>  | lt  | b   | a   | y   | x   |     |     |     |     | <>  | <>  |
| GC            | <>   | <>  | lt  | b   | a   | y   | x   |     |     |     |     | <>  | <>  |
| SMS           | <>   |     |     | 1   | 2   |     |     |     |     |     |     |     |     |
| SMD           | <>   | <>  | c   | a   | b   | x   | y   | z   |     |     |     |     |     |
| Saturn        | <>   | ?   | ?   | ?   | ?   | ?   | ?   | ?   | ?   | ?   | ?   | ?   | ?   |
| Keyboard      | <>   | f1  | f2  | z   | x   | v   | b   | a   | s   | d   | f   | q   | e   |

| System\Bit    | 10-17 | 18-1f | 20-27 | 28-2f | 30-33 | 34-37 | 38-3b | 3c-3f |
|---------------|-------|-------|-------|-------|-------|-------|-------|-------|
| Plox Standard | a0x   | a0y   | a1x   | a1y   | a0tl  | a0tr  | a1tl  | a1tr  |
| NES           |       |       |       |       |       |       |       |       |
| SNES          |       |       |       |       |       |       |       |       |
| N64           | <>    | <>    | <>    | <>    |       |       |       |       |
| GC            | <>    | <>    | <>    | <>    |       |       |       |       |
| SMS           |       |       |       |       |       |       |       |       |
| SMD           |       |       |       |       |       |       |       |       |
| Saturn        |       |       |       |       |       |       |       |       |
| Keyboard      | kc0   | kc1   | kc2   | kc3   |       |       |       |       |

