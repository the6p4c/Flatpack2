#include <mcp_can.h>
#include <SPI.h>

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

void printMessage(char *direction, long unsigned int id, unsigned char len, unsigned char buf[]) {
	char output[256];

	snprintf(output, 256, "%s ID: 0x%.8lX Length: %1d Data:", direction, id, len);
	Serial.print(output);

	for (int i = 0; i < len; ++i) {
		snprintf(output, 256, " 0x%.2X", buf[i]);
		Serial.print(output);
	}

	Serial.println();
}

void loop() {
	while (Serial.available()) {
		// IDIDIDID NB DD DD DD DD DD DD DD DD

		String s = Serial.readString();

		long unsigned int txID = strtol(s.substring(0, 8).c_str(), nullptr, 16);
		unsigned char len = strtol(s.substring(9, 10).c_str(), nullptr, 10);
		unsigned char txBuf[8] = {0};

		Serial.println();

		for (int i = 0; i < len; ++i) {
			txBuf[i] = strtol(s.substring(11 + i * 3, 14 + i * 3).c_str(), nullptr, 16);
		}

		CAN.sendMsgBuf(txID, 1, len, txBuf);
		
		printMessage("TX", txID, len, txBuf);
	}

	// INT pin is active low
	if (!digitalRead(CAN_INT_PIN)) {
		long unsigned int rxID;
		unsigned char len = 0;
		unsigned char rxBuf[8];

		CAN.readMsgBuf(&rxID, &len, rxBuf);

		// Limit ID to lowest 29 bits
		rxID &= 0x1FFFFFFF;

		printMessage("RX", rxID, len, rxBuf);
	}
}
