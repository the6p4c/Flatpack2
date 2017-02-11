# Conventions
RX is PSU -> Arduino
TX is Arduino -> PSU

# ID Format
IDs seem to be formatted as follows:
`0x05AABBBB`
| Field | Usage |
| --- | --- |
| AA | Either `00` when an operation needs to ocurr when an ID has not yet been transmitted, a supply's ID (`01`, `02`, ... , `FE`), or the broadcast ID `FF` |
| BBBB | True message ID |

# Messages
## `0x0500XXXX`
Sent approximately every 2 seconds. `XXXX` are the last 4 digits of the serial number.
Possibly power supply asking to be logged in?
```
RX ID: 0x05001554
   Length: 8
   Data: 0x1B 0x13 0x20 0x71 0x18 0x15 0x54 0x00
                S1   S2   S3   S4   S5   S6  
```
| Field | Usage |
| --- | --- |
| S1-S6 | Power supply's serial number |

## `0x050140XX`
Sent by the power supply after log in. Contains information about the state of the supply.
When the power supply is operating normally, `X = 04`. When the supply has a warning, `X = 08`. When an alarm is present, `X = 0C`. When the supply is in walk mode (voltage ramping up) `X = 10`.

```
RX ID: 0x050140XX
   Length: 8
   Data: 0x1F 0x00 0x00 0x8F 0x11 0xEB 0x00 0x24
           TI   C0   C1   O0   O1   I0   I1   TO
```
All temperatures in deg C. Current in deciamps. Output voltage in centivolts. Input voltage in volts.
| Field | Usage |
| --- | --- |
| TI | Intake temperature |
| C0 | Current low byte |
| C1 | Current high byte |
| O0 | Output voltage low byte |
| O1 | Output voltage high byte |
| I0 | Input voltage low byte |
| I1 | Input voltage high byte |
| TO | Output temperature |

## `0x05004804`
Sent to log in to a power supply. Unknown if address is a broadcast or specific message to single PSU.
```
TX ID: 0x05004804
   Length: 8
   Data: 0x13 0x20 0x71 0x18 0x15 0x54 0x00 0x00
           S1   S2   S3   S4   S5   S6
```
| Field | Usage |
| --- | --- |
| S1-S6 | Power supply's serial number |

## `0x05014400`
Sent approximately every 15 seconds. Likely power supply introducing itself to the CAN bus network.
```
RX ID: 0x05014400
   Length: 8
   Data: 0x13 0x20 0x71 0x18 0x15 0x54 0x00 0x00
           S1   S2   S3   S4   S5   S6
```
| Field | Usage |
| --- | --- |
| S1-S6 | Power supply's serial number |

## `0x05019C00`
Sent to change the default voltage of a supply. Voltage is in centivolts, i.e. 48.52V is 4852. Likely a direct command to the supply with ID 1. Doesn't seem to take effect until supply is logged off?
```
TX ID: 0x05019C00
   Length: 5
   Data: 0x29 0x15 0x00 0x94 0x11
                          V0   V1
```
| Field | Usage |
| --- | --- |
| V0 | Voltage low byte |
| V1 | Voltage high byte |
