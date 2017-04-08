<!--
<table>
	<tr>
		<td><b>Byte</b></td> <td>0</td> <td>1</td> <td>2</td> <td>3</td> <td>4</td> <td>5</td> <td>6</td> <td>7</td>
	</tr>
	<tr>
		<td><b>Value</b></td> <td>Byte 0</td> <td>Byte 1</td> <td>Byte 2</td> <td>Byte 3</td> <td>Byte 4</td> <td>Byte 5</td> <td>Byte 6</td> <td>Byte 7</td>
	</tr>
</table>
-->

# Conventions in This Document
RX is a message from the power supply to the transceiver  
TX is a message from the transceiver to the power supply

# Hardware
The Flatpack2's CAN bus runs at 125kbit/s, using an extended ID field. The bus is relative to the PSU's negative output rail. Failure to connect the negative of the supply with the ground of your CAN transceiver will likely blow the transceiver up (this took me 3 MCP2551s to realize).

# Messages

## (RX) Log in request (??), `0x0500XXXX`
Sent approximately every two seconds. `XXXX` are usually the last four digits of the power supply's serial number. (Doesn't always exactly match - supply ending with 5418 sends 1418)

After approximately 15 seconds, the power supply will log out again if it does not recieve another log in message.

<table>
	<tr>
		<td><b>Byte</b></td> <td>0</td> <td>1</td> <td>2</td> <td>3</td> <td>4</td> <td>5</td> <td>6</td> <td>7</td>
	</tr>
	<tr>
    	<td><b>Value</b></td> <td><code>0x1B</code></td> <td colspan='6'>Power supply's serial number</td> <td><code>0x00</code></td>
	</tr>
</table>

## (TX) Log in, `0x050048XX`
Sent to log into a power supply. Unknown if address is a broadcast or a specific message to a single power supply.

The ID of the supply is set by the last byte of the address, where `XX = ID * 4`. The ID ranges from `0x01` to `0x3F` (resulting in a range for `XX` of `0x04` to `0xFC`).

<table>
	<tr>
		<td><b>Byte</b></td> <td>0</td> <td>1</td> <td>2</td> <td>3</td> <td>4</td> <td>5</td> <td>6</td> <td>7</td>
	</tr>
	<tr>
		<td><b>Value</b></td> <td colspan='6'>Power supply's serial number</td> <td><code>0x00</code></td> <td><code>0x00</code></td>
	</tr>
</table>

## (RX) Status, `0x05XX40YY`
Sent by the power supply after it is logged into, and contains information about the current state of the power supply.

`XX` is the power supply's ID.

| Value of `YY` | Power supply state |
| --- | --- |
| `0x04` | Normal operation |
| `0x08` | Warning |
| `0x0C` | Alarm |
| `0x10` | Walk in (voltage ramping up) |

Current, output voltage and input voltage are stored in little endian (LSB first). All temperatures are in degrees Celsius. Current is in deciamps (i.e. 21.2A is 212). Output voltage is in centivolts (i.e. 48.52V is 4852). Input voltage is in volts.

<table>
	<tr>
		<td><b>Byte</b></td> <td>0</td> <td>1</td> <td>2</td> <td>3</td> <td>4</td> <td>5</td> <td>6</td> <td>7</td>
	</tr>
	<tr>
		<td><b>Value</b></td> <td>Intake temperature</td> <td colspan='2'>Current</td> <td colspan='2'>Output voltage</td> <td colspan='2'>Input voltage</td> <td>Output temperature</td>
	</tr>
</table>

## (RX) CAN Bus network introduction, `0x05XX4400`
Sent approximately every 15 seconds. Seems to be the power supply introducing itself to the CAN bus network.

`XX` is the power supply's ID.

<table>
	<tr>
		<td><b>Byte</b></td> <td>0</td> <td>1</td> <td>2</td> <td>3</td> <td>4</td> <td>5</td> <td>6</td> <td>7</td>
	</tr>
	<tr>
		<td><b>Value</b></td> <td colspan='6'>Power supply's serial number</td> <td><code>0x00</code></td> <td><code>0x00</code></td>
	</tr>
</table>

## (TX) Set default voltage, `0x05XX9C00`
Sent to the power supply to set its default voltage. Does not take effect until the supply is logged out. If the supply is logged in when the command is sent, the voltage is set when the log in times out. If it is not logged in, the voltage will be set when the supply logs in then times out.

`XX` is the power supply's ID.

The voltage is stored in little-endian and is in centivolts (i.e. 48.52V is 4852).

<table>
	<tr>
		<td><b>Byte</b></td> <td>0</td> <td>1</td> <td>2</td> <td>3</td> <td>4</td>
	</tr>
	<tr>
		<td><b>Value</b></td> <td><code>0x29</code></td> <td><code>0x15</code></td> <td><code>0x00</code></td> <td colspan='2'>New voltage</td>
	</tr>
</table>

## (RX) Alarms/warnings information, `0x05XXBFCC`
Sent by the power supply in response to recieving an `0x05XXBFCC` message. Contains information about any alarms or warnings present, depending on the contents of the request message.

`XX` is the power supply's ID.

<table>
	<tr>
		<td><b>Byte</b></td> <td>0</td> <td>1</td> <td>2</td> <td>3</td> <td>4</td> <td>5</td> <td>6</td>
	</tr>
	<tr>
		<td><b>Value</b></td> <td><code>0x0E</code></td> <td><code>0x04</code> (warnings), <code>0x08</code> (alarms)</td> <td><code>0x00</code></td> <td>Warning/alarm bit field byte 1</td> <td>Warning/alarm bit field byte 2</td> <td><code>0x00</code></td> <td><code>0x00</code></td>
	</tr>
</table>

### Warnings/Alarms
Bit 0 is the LSB.

| Bit | Warning/alarm bit field byte 1 | Warning/alarm bit field 2 |
| --- | --- | --- |
| 0 | OVS Lock Out | Internal Voltage |
| 1 | Mod Fail Primary | Module Fail |
| 2 | Mod Fail Secondary | Mod Fail Secondary |
| 3 | High Mains | Fan 1 Speed Low |
| 4 | Low Mains | Fan 2 Speed Low |
| 5 | High Temp | Sub Mod1 Fail |
| 6 | Low Temp | Fan 3 Speed Low |
| 7 | Current Limit | Inner Volt |

## (TX) Alarms/warnings information request, `0x05XXBFCC`
Sent to the power supply to request information on the current warnings and alarms. Should be sent after recieving an `0x05XX40YY` message where `YY = 08` or `YY = 0C` (i.e a warning or alarm is present). Byte 2 determines if warning or alarm information is returned.

<table>
	<tr>
		<td><b>Byte</b></td> <td>0</td> <td>1</td> <td>2</td>
	</tr>
	<tr>
		<td><b>Value</b></td> <td><code>0x08</code></td> <td><code>0x04</code> (warnings), <code>0x08</code> (alarms)</td> <td><code>0x00</code></td>
	</tr>
</table>
