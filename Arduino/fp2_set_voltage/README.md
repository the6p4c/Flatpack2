# How to use
You'll need the following library (simply install it like any other Arduino library):  
https://github.com/coryjfowler/MCP_CAN_lib

Then, download this sketch. You'll have to modify the `#define VOLTAGE ...` line to indicate what voltage you require. This value is in centivolts, so you'll have to convert your desired voltage into that unit. For example, if you want the supply to output 47.13 V, change the line to `#define VOLTAGE 4713`.

You may also have to change the `const int CAN_CS_PIN = ...;` and `const int CAN_INT_PIN = ...;` lines to match how your transceiver is wired up. If you're using the transceiver as specified in this repository, you won't need to change anything here.

If your transceiver doesn't use a MCP2515, then you might have to rewrite some of the sketch to interface correctly. The specifics of the protocol are in [this](https://github.com/The6P4C/Flatpack2/tree/master/Protocol.md) file and how they're used should hopefully be pretty clear by reading the sketch.

# Changing the voltage
After the supply is connected to your Arduino (CAN bus and common ground), you should simply be able to turn on the PSU, upload the sketch to your Arduino, and open the serial monitor to check the process is running smoothly. You should see the messages "Entering Configuration Mode Successful!", "Setting Baudrate Successful!" and "MCP2515 initialized successfully.". After a moment, the message "Starting set" will appear. Your screen will probably fill with messages that look similar to (but not exactly like) this: "ID: 0x05014004 Length: 8 Data: 0x17 0x00 0x00 0xD1 0x12 0xEE 0x00 0x17". After a while, the stream of messages will slow down to one message around every 3-4 seconds.

When this happens, turn off the power supply's AC power, and disconnect the USB, then the CAN bus connection, then disconnect the common ground. Give the supply about 2 minutes to properly shut down and drain its internal caps, then reconnect only the AC power and measure the DC output voltage. It should, with luck, measure as the voltage you set.
