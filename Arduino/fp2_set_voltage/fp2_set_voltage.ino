#include <mcp_can.h>
#include <SPI.h>

#define VOLTAGE 4800

const int CAN_CS_PIN = 10;
const int CAN_INT_PIN = 2;

MCP_CAN CAN(CAN_CS_PIN);

void setup() {
	Serial.begin(115200);

	pinMode(CAN_CS_PIN, OUTPUT);
	pinMode(CAN_INT_PIN, INPUT);

	if (CAN.begin(MCP_ANY, CAN_125KBPS, MCP_16MHZ) == CAN_OK) {
		Serial.println("MCP2515 initialized successfully.");
	} else {
		Serial.println("Failed to initialize MCP2515. Halting.");
		while(1);
	}

	CAN.setMode(MCP_NORMAL);
}

void printMessage(uint32_t rxID, uint8_t len, uint8_t rxBuf[]) {
	char output[256];

	snprintf(output, 256, "ID: 0x%.8lX Length: %1d Data:", rxID, len);
	Serial.print(output);

	for (int i = 0; i < len; ++i) {
		snprintf(output, 256, " 0x%.2X", rxBuf[i]);
		Serial.print(output);
	}

	Serial.println();
}

bool done = false;
void loop() {
	// Active low
	if (!digitalRead(CAN_INT_PIN)) {
		uint32_t rxID;
		uint8_t len = 0;
		uint8_t rxBuf[8];

		CAN.readMsgBuf((unsigned long *)&rxID, &len, rxBuf);

		// Limit ID to lowest 29 bits (extended CAN)
		rxID &= 0x1FFFFFFF;

		printMessage(rxID, len, rxBuf);

		if (!done && (rxID & 0xFFFF0000) == 0x05000000) {
			Serial.println("Starting set");

			uint8_t serialNumber[6];

			for (int i = 0; i < 6; ++i) {
				serialNumber[i] = rxBuf[i + 1];
			}

			uint8_t logInTxBuf[8] = { 0 };

			for (int i = 0; i < 6; ++i) {
				logInTxBuf[i] = serialNumber[i];
			}

			CAN.sendMsgBuf(0x05004804, 1, 8, logInTxBuf);

			delay(100);

			uint8_t voltageSetTxBuf[5] = { 0x29, 0x15, 0x00, VOLTAGE & 0xFF, (VOLTAGE >> 8) & 0xFF };

			CAN.sendMsgBuf(0x05019C00, 1, 5, voltageSetTxBuf);

			Serial.println("Set completed");

			done = true;
		}
	}
}
