// alias for `#include "SSD1306Wire.h"`
#include "SSD1306.h"
#include "Credentials.h"
// Initialize the OLED display using Wire library
SSD1306  display(0x3c, 5, 4);

#include <PubSubClient.h>
// Update these with values suitable for your network.
const char* ssid = SSID;
const char* password = PASSWORD;
const char* mqtt_server = "192.168.0.112";


void setup() {
	Serial.begin(115200);
	// Initialising the UI will init the display too.
	display.init();
	display.flipScreenVertically();
	display.setFont(ArialMT_Plain_10);
}


void loop() {
	// clear the display
	display.clear();
	display.setTextAlignment(TEXT_ALIGN_RIGHT);
	display.drawString(128, 0, String(millis()));
	// write the buffer to the display
	display.display();
	delay(15);
}

