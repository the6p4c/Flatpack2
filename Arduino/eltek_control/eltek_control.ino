#include <mcp_can.h>
#include <SPI.h>

const int CAN_CS_PIN = 10;
const int CAN_INT_PIN = 2;

MCP_CAN CAN(CAN_CS_PIN);

bool serialNumberPresent = false;
uint8_t serialNumber[6];

unsigned long lastLogInTime;

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

void printMessage(long unsigned int rxID, unsigned char len, unsigned char rxBuf[]) {
	char output[256];

	snprintf(output, 256, "ID: 0x%.8lX Length: %1d Data:", rxID, len);
	Serial.print(output);

	for (int i = 0; i < len; ++i) {
		snprintf(output, 256, " 0x%.2X", rxBuf[i]);
		Serial.print(output);
	}

	Serial.println();
}

void logIn() {
	if (serialNumberPresent) {
		uint8_t txBuf[8] = {0, 0, 0, 0, 0, 0, 0, 0};

		for (int i = 0; i < 6; ++i) {
			txBuf[i] = serialNumber[i];
		}

		CAN.sendMsgBuf(0x05004804, 1, 8, txBuf);
	}
}

void processIdentificationMessage(long unsigned int rxID, int len, unsigned char rxBuf[]) {
	Serial.println("--------");
	Serial.print("Found power supply, logging in: ");

	char output[3];

	for (int i = 0; i < 6; ++i) {
		serialNumber[i] = rxBuf[i];

		snprintf(output, 3, "%.2X", serialNumber[i]);
		Serial.print(output);
	}

	Serial.println();
	Serial.println("--------");

	serialNumberPresent = true;

	logIn();
}

void processStatusMessage(long unsigned int rxID, int len, unsigned char rxBuf[]) {
	int intakeTemperature = rxBuf[0];
	float current = 0.1f * (rxBuf[1] | (rxBuf[2] << 8));
	float outputVoltage = 0.01f * (rxBuf[3] | (rxBuf[4] << 8));
	int inputVoltage = rxBuf[5] | (rxBuf[6] << 8);
	int outputTemperature = rxBuf[7];

	char output[256];
	
	Serial.println("--------");
	Serial.println("Charger status message:");

	Serial.print("Intake temperature: ");
	Serial.print(intakeTemperature);
	Serial.println(" deg C");

	Serial.print("Current: ");
	Serial.print(current);
	Serial.println("A");

	Serial.print("Output voltage: ");
	Serial.print(outputVoltage);
	Serial.println("V");

	Serial.print("Input voltage: ");
	Serial.print(inputVoltage);
	Serial.println("V");

	Serial.print("Output temperature: ");
	Serial.print(outputTemperature);
	Serial.println(" deg C");

	if (rxID == 0x05014010) {
		Serial.println("Currently in walk in (voltage ramping up)");
	}

	bool hasWarning = rxID == 0x05014008;
	bool hasAlarm = rxID == 0x0501400C;

	if (hasWarning) {
		Serial.println("WARNING");
	} else if (hasAlarm) {
		Serial.println("ALARM");
	}

	if (hasWarning || hasAlarm) {
		uint8_t txBuf[3] = {0x08, hasWarning ? 0x04 : 0x08, 0x00};
		CAN.sendMsgBuf(0x0501BFFC, 1, 3, txBuf);
	}

	Serial.println("--------");
}

void processAlarmWarningMessage(long unsigned int rxID, int len, unsigned char rxBuf[]) {
	bool isWarning = rxBuf[1] == 0x04;

	Serial.println("--------");
	if (isWarning) {
		Serial.print("Warnings:");
	} else {
		Serial.print("Alarms:");
	}

	uint8_t alarms0 = rxBuf[3];
	uint8_t alarms1 = rxBuf[4];


	if (alarms0 & 0x80) {
		Serial.print(" ");
		Serial.print("CURRENT_LIMIT");
	}

	if (alarms0 & 0x40) {
		Serial.print(" ");
		Serial.print("LOW_TEMP");
	}

	if (alarms0 & 0x20) {
		Serial.print(" ");
		Serial.print("HIGH_TEMP");
	}

	if (alarms0 & 0x10) {
		Serial.print(" ");
		Serial.print("LOW_MAINS");
	}

	if (alarms0 & 0x08) {
		Serial.print(" ");
		Serial.print("HIGH_MAINS");
	}

	if (alarms0 & 0x04) {
		Serial.print(" ");
		Serial.print("MOD_FAIL_SECONDARY");
	}

	if (alarms0 & 0x02) {
		Serial.print(" ");
		Serial.print("MOD_FAIL_PRIMARY");
	}

	if (alarms0 & 0x01) {
		Serial.print(" ");
		Serial.print("OVS_LOCK_OUT");
	}

	if (alarms1 & 0x80) {
		Serial.print(" ");
		Serial.print("INNER_VOLT");
	}

	if (alarms1 & 0x40) {
		Serial.print(" ");
		Serial.print("FAN3_SPEED_LOW");
	}

	if (alarms1 & 0x20) {
		Serial.print(" ");
		Serial.print("SUB_MOD1_FAIL");
	}

	if (alarms1 & 0x10) {
		Serial.print(" ");
		Serial.print("FAN2_SPEED_LOW");
	}

	if (alarms1 & 0x08) {
		Serial.print(" ");
		Serial.print("FAN1_SPEED_LOW");
	}

	if (alarms1 & 0x04) {
		Serial.print(" ");
		Serial.print("MOD_FAIL_SECONDARY");
	}

	if (alarms1 & 0x02) {
		Serial.print(" ");
		Serial.print("MODULE_FAIL");
	}

	if (alarms1 & 0x01) {
		Serial.print(" ");
		Serial.print("INTERNAL_VOLTAGE");
	}

	Serial.println();
	Serial.println("--------");
}

void loop() {
	// INT pin is active low
	if (!digitalRead(CAN_INT_PIN)) {
		long unsigned int rxID;
		unsigned char len = 0;
		unsigned char rxBuf[8];

		CAN.readMsgBuf(&rxID, &len, rxBuf);

		// Limit ID to lowest 29 bits
		rxID &= 0x1FFFFFFF;

		printMessage(rxID, len, rxBuf);

		switch (rxID) {
		case 0x05014400:
			processIdentificationMessage(rxID, len, rxBuf);
			break;

		case 0x05001554:
			Serial.println("--------");
			Serial.println("Recieved log in request, logging in");
			Serial.println("--------");
			logIn();
			break;

		case 0x0501BFFC:
			processAlarmWarningMessage(rxID, len, rxBuf);
			break;

		case 0x05014004:
		case 0x05014008:
		case 0x0501400C:
		case 0x05014010:
			processStatusMessage(rxID, len, rxBuf);
			break;
		}
	}
}
