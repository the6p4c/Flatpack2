Electronics
===========
CAN bus running at 125Kbit/s

Messages
========
Every 10 seconds PSU sends its serial number
`RX EID: 0x05014400 Data: 0x13 0x20 0x71 0x18 0x15 0x54 0x00 0x00`
Serial number followed by 0x00 0x00

Log in with serial number
`TX EID: 0x05004804 Data: 0x13 0x20 0x71 0x18 0x15 0x54 0x00 0x00`
Data is serial number from other packet

After log in PSU sends some status information
`RX EID: 0x05014004 Data: 0x1B 0x00 0x00 0xDC 0x14 0xF1 0x00 0x23`
`T0 C0 C1 V0 V1 I0 ?? ??`
T0 - Temperature
C1C0 - Current in deciamps
V0V1 - Voltage in centivolts
I0 - Input voltage in volts

Device Idea?
============

Log in to first PSU to advertise itself
Turn on a connected LED
Send voltage set message (maybe multiple times?)
Turn on a 'completed' LED after 60 seconds
Log out
Voltage should jump to new set point
