#include <mcp_can.h>
#include <SPI.h>

uint8_t serialNumber[6] = {0x13, 0x20, 0x71, 0x18, 0x15, 0x54};
const int VOLTAGE = 4620;

const int CAN_CS_PIN = 10;
const int CAN_INT_PIN = 2;

MCP_CAN CAN(CAN_CS_PIN);

void logIn() {
	uint8_t txBuf[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	for (int i = 0; i < 6; ++i) {
		txBuf[i] = serialNumber[i];
	}

	CAN.sendMsgBuf(0x05004804, 1, 8, txBuf);
}

void setVoltage() {
	uint8_t txBuf[5] = {0x29, 0x15, 0x00, VOLTAGE & 0xFF, (VOLTAGE >> 8) & 0xFF};
	CAN.sendMsgBuf(0x05019C00, 1, 5, txBuf);
}

void setup() {
	Serial.begin(115200);

	pinMode(CAN_CS_PIN, OUTPUT);
	pinMode(CAN_INT_PIN, INPUT);

	if (CAN.begin(MCP_ANY, CAN_125KBPS, MCP_16MHZ) == CAN_OK) {
		Serial.println("MCP2515 initialized successfully.");
		Serial.println();
	} else {
		Serial.println("Failed to initialize MCP2515. Halting.");
		while(1);
	}

	CAN.setMode(MCP_NORMAL);

	bool loggedIn = false;

	while (!loggedIn) {
		if (!digitalRead(CAN_INT_PIN)) {
			long unsigned int rxID;
			unsigned char len = 0;
			unsigned char rxBuf[8];

			CAN.readMsgBuf(&rxID, &len, rxBuf);

			// Limit ID to lowest 29 bits
			rxID &= 0x1FFFFFFF;

			if (rxID == 0x05000000 | serialNumber[4] << 8 | serialNumber[5]) {
				Serial.println("Recieved log in message from power supply. Logging in...");
				Serial.println();
				loggedIn = true;
			}
		}
	}

	bool loggedInSuccessfully = false;
	unsigned long lastLogInTime = 0;

	while (!loggedInSuccessfully) {
		if (millis() - lastLogInTime > 100) {
			logIn();
			lastLogInTime = millis();
		}

		if (!digitalRead(CAN_INT_PIN)) {
			long unsigned int rxID;
			unsigned char len = 0;
			unsigned char rxBuf[8];

			CAN.readMsgBuf(&rxID, &len, rxBuf);

			// Limit ID to lowest 29 bits
			rxID &= 0x1FFFFFFF;

			if ((rxID & 0xFFFFFF00) == 0x05014000) {
				Serial.println("Successfully logged in. Setting voltage...");
				Serial.println();
				loggedInSuccessfully = true;
			}
		}
	}

	setVoltage();

	Serial.println("Voltage change message sent. Leave power supply connected for 30 seconds,");
	Serial.println("and new voltage will be set. Disconnect + reconnect AC to confirm voltage");
	Serial.println("set correctly.");
}

void loop() {

}
