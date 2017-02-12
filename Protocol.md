# Conventions In This Document
RX is Power Supply -> Transceiver  
TX is from Transceiver -> Power Supply

# Hardware Layer
The Flatpack2's CAN bus runs at 125kbit/s, using an extended ID field. The bus is relative to the power supply's output negative. Failure to connect the negative of the supply with the ground of your CAN transceiver will likely blow the transceiver up. (Side note: this took me 3 MCP2551s to realize.)

# ID Format
CAN bus IDs seem to be formatted as follows:

```
0x05AABBBB
```

| Field | Usage |
| --- | --- |
| `AA` | `00` if the message is a broadcast to all power supplies on the CAN bus, otherwise the supply's ID. |
| `BBBB` | The true message ID |

# Messages
## RX `0x0500XXXX`
Sent approximately every two seconds. `XXXX` are the last four digits of the power supply's serial number.  
Possibly the power supply asking to be logged in?

```
Length: 8
  Data: 0x1B _S1_ _S2_ _S3_ _S4_ _S5_ _S6_ 0x00
```

| Field | Usage |
| --- | --- |
| `S1-S6` | Power supply's serial number |

## TX `0x05004804`
Sent to log into a power supply. Unknown if address is a broadcase or specific message to a single power supply.

```
Length: 8
  Data: _S1_ _S2_ _S3_ _S4_ _S5_ _S6_ 0x00 0x00
```

| Field | Usage |
| --- | --- |
| `S1-S6` | Power supply's serial number |

## RX `0x050140XX`
Sent by the supply after log in, and contains information about the state of the power supply. When the supply is operating normally, `XX = 0x04`. When the supply has a warning, `XX = 0x08`. When an alarm is present, `XX = 0x0C`. When the supply is in walk in mode (voltage ramping up), `XX = 0x10`.  
All temperatures in degrees Celsius. Current in deciamps, i.e. 21.2A is 212. Output voltage in centivolts, i.e. 48.52V is 4852. Input voltage in volts.

```
  Length: 8
    Data: _TI_ _C1_ _C2_ _O1_ _O2_ _I1_ _I2_ _TO_
```

| Field | Usage |
| --- | --- |
| `TI` | Intake temperature |
| `C1` | Current low byte |
| `C2` | Current high byte |
| `O1` | Output voltage low byte |
| `O2` | Output voltage high byte |
| `I1` | Input voltage low byte |
| `I2` | Input voltage high byte |
| `TO` | Output temperature |

## RX, `0x05014400`
Sent approximately every 15 seconds. Likely the power supply introducing itself to the CAN bus network.

```
Length: 8
  Data: _S1_ _S2_ _S3_ _S4_ _S5_ _S6_ 0x00 0x00
```

| Field | Usage |
| --- | --- |
| `S1-S6` | Power supply's serial number |

## TX, `0x05019C00`
Sent to the power supply to change its default voltage. Seems to be a direct command to the supply with ID 1. Does not seem to take effect until the supply is logged off for some amount of time.  
Voltage is in centivolts, i.e. 48.52V is 4852.

```
Length: 5
  Data: 0x29 0x15 0x00 _V1_ V2_
```

| Field | Usage |
| --- | --- |
| `V1` | Voltage low byte |
| `V2` | Voltage high byte |

## RX, `0x0501BFCC`
Sent by the supply in response to recieving an `0x0501BFCC` message. Contains information about any alarms or warnings present, depending on the contents of the message initially sent.

```
Length: 7
  Data: 0x0E _T1_ 0x00 _E1_ _E2_ 0x00 0x00
```

| Field | Usage |
| --- | --- |
| `T1` | `0x04` if message contains warning information, `0x08` if message contains alarm information |
| `E1-E2` | Warning/alarm bit field |

### Warnings/Alarms
| Bit | `E1` | `E2` |
| --- | --- | --- |
| 0 | OVS Lock Out | Internal Voltage |
| 1 | Mod Fail Primary | Module Fail |
| 2 | Mod Fail Secondary | Mod Fail Secondary |
| 3 | High Mains | Fan 1 Speed Low |
| 4 | Low Mains | Fan 2 Speed Low |
| 5 | High Temp | Sub Mod1 Fail |
| 6 | Low Temp | Fan 3 Speed Low |
| 7 | Current Limit | Inner Volt |

## TX, `0x0501BFFC`
Sent to the power supply to get information on currently active alarms or warnings. Should only be sent after recieving a `0x050140XX` where `X = 08` or `X = 0C`.

```
Length: 3
  Data: 0x08 _T1_ 0x00
```

| Field | Usage |
| --- | --- |
| `T1` | `0x04` for warning information, `0x08` for alarm information |
