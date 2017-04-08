#include <mcp_can.h>
#include <SPI.h>

const int CAN_CS_PIN = 10;
const int CAN_INT_PIN = 2;

const char *alarms0Strings[] = {"OVS_LOCK_OUT", "MOD_FAIL_PRIMARY", "MOD_FAIL_SECONDARY", "HIGH_MAINS", "LOW_MAINS", "HIGH_TEMP", "LOW_TEMP", "CURRENT_LIMIT"};
const char *alarms1Strings[] = {"INTERNAL_VOLTAGE", "MODULE_FAIL", "MOD_FAIL_SECONDARY", "FAN1_SPEED_LOW", "FAN2_SPEED_LOW", "SUB_MOD1_FAIL", "FAN3_SPEED_LOW", "INNER_VOLT"};

MCP_CAN CAN(CAN_CS_PIN);

bool serialNumberReceived = false;
uint8_t serialNumber[6];

unsigned long lastLogInTime = 0;

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

void logIn() {
	Serial.println("--------");
	Serial.println("Logging in.");
	Serial.println("--------");

	uint8_t txBuf[8] = { 0 };

	for (int i = 0; i < 6; ++i) {
		txBuf[i] = serialNumber[i];
	}

	CAN.sendMsgBuf(0x05004804, 1, 8, txBuf);
}

void processLogInRequest(uint32_t rxID, uint8_t len, uint8_t rxBuf[]) {
	Serial.println("--------");
	Serial.print("Found power supply ");

	char output[3];

	for (int i = 0; i < 6; ++i) {
		serialNumber[i] = rxBuf[i + 1];

		snprintf(output, 3, "%.2X", serialNumber[i]);
		Serial.print(output);
	}

	serialNumberReceived = true;

	Serial.println();
	Serial.println("--------");
}

void processStatusMessage(uint32_t rxID, uint8_t len, uint8_t rxBuf[]) {
	int intakeTemperature = rxBuf[0];
	float current = 0.1f * (rxBuf[1] | (rxBuf[2] << 8));
	float outputVoltage = 0.01f * (rxBuf[3] | (rxBuf[4] << 8));
	int inputVoltage = rxBuf[5] | (rxBuf[6] << 8);
	int outputTemperature = rxBuf[7];

	char output[256];
	
	Serial.println("--------");
	Serial.println("Status message:");

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

void processWarningOrAlarmMessage(uint32_t rxID, uint8_t len, uint8_t rxBuf[]) {
	bool isWarning = rxBuf[1] == 0x04;

	Serial.println("--------");
	if (isWarning) {
		Serial.print("Warnings:");
	} else {
		Serial.print("Alarms:");
	}

	uint8_t alarms0 = rxBuf[3];
	uint8_t alarms1 = rxBuf[4];

	for (int i = 0; i < 8; ++i) {
		if (alarms0 & (1 << i)) {
			Serial.print(" ");
			Serial.print(alarms0Strings[i]);
		}

		if (alarms1 & (1 << i)) {
			Serial.print(" ");
			Serial.print(alarms1Strings[i]);
		}
	}

	Serial.println();
	Serial.println("--------");;
}

void loop() {
	// Log in every second
	if (serialNumberReceived && millis() - lastLogInTime > 1000) {
		logIn();

		lastLogInTime = millis();
	}

	// Active low
	if (!digitalRead(CAN_INT_PIN)) {
		uint32_t rxID;
		uint8_t len = 0;
		uint8_t rxBuf[8];

		CAN.readMsgBuf((unsigned long *)&rxID, &len, rxBuf);

		// Limit ID to lowest 29 bits (extended CAN)
		rxID &= 0x1FFFFFFF;

		printMessage(rxID, len, rxBuf);

		if (!serialNumberReceived && (rxID & 0xFFFF0000) == 0x05000000) {
			processLogInRequest(rxID, len, rxBuf);
		} else if ((rxID & 0xFFFFFF00) == 0x05014000) {
			processStatusMessage(rxID, len, rxBuf);
		} else if (rxID == 0x0501BFFC) {
			processWarningOrAlarmMessage(rxID, len, rxBuf);
		}
	}
}
