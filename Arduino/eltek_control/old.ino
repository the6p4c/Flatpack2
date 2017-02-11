#include <mcp_can.h>
#include <SPI.h>

#define CAN_CS_PIN 10
#define CAN_INT_PIN 2

MCP_CAN CAN(CAN_CS_PIN);

void setup() {
  Serial.begin(115200);

  if (CAN.begin(MCP_ANY, CAN_125KBPS, MCP_16MHZ) == CAN_OK) {
    Serial.println("Initialization succeeded");
  } else {
    Serial.println("Initialization failed");
    while(1);
  }

  CAN.setMode(MCP_NORMAL);
  pinMode(CAN_INT_PIN, INPUT);
}

unsigned long lastLogOn = 0;

void loop() {
  if (millis() - lastLogOn >= 1000) {
    char logOn[8] = { 0x13, 0x20, 0x71, 0x18, 0x15, 0x54, 0x00, 0x00 };
    CAN.sendMsgBuf(0x05004804, 1, 8, logOn);

    delay(100);

    // in centivolts (i.e. 48.52V is 4852)
    int voltage = 4500;
    char changeVoltage[5] = { 0x29, 0x15, 0x00, voltage & 0xFF, (voltage >> 8) & 0xFF };
    CAN.sendMsgBuf(0x05019C00, 1, 5, changeVoltage);

    lastLogOn = millis();
  }

  if (!digitalRead(CAN_INT_PIN)) {
    long unsigned int rxID;
    unsigned char len = 0;
    unsigned char rxBuf[8];

    char output[128];

    CAN.readMsgBuf(&rxID, &len, rxBuf);

    switch (rxID & 0x1FFFFFFF) {
      default:
        snprintf(output, 128, "ID: 0x%.8lX Length: %1d Data:", (rxID & 0x1FFFFFFF), len);
        Serial.print(output);

        for (int i = 0; i < len; ++i) {
          snprintf(output, 128, " 0x%.2X", rxBuf[i]);
          Serial.print(output);
        }

        Serial.println();

        break;
    }
  }
}
