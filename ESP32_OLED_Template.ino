// alias for `#include "SSD1306Wire.h"`
#include "SSD1306.h"
#include "Credentials.h"
#include "WiFi.h"
// Initialize the OLED display using Wire library
SSD1306  display(0x3c, 5, 4);

#include <PubSubClient.h>
// Update these with values suitable for your network.

const char* mqtt_server = MQTT_SERVER_IP;
WiFiClient espClient;
PubSubClient client(espClient);
String temperature;
unsigned long lastRefresh = 0;
unsigned long refresh = 0;
unsigned long refreshScreenPeriod = 1000;

void setup_wifi() {

	delay(50);
	// We start by connecting to a WiFi network
	Serial.println();
	Serial.print("Connecting to ");
	Serial.println(SSID);

	WiFi.begin(SSID, PASSWORD);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	Serial.println("");
	Serial.println("WiFi connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
	Serial.print("Message arrived [");
	Serial.print(topic);
	Serial.print("] ");
	for (int i = 0; i < length; i++) {
		Serial.print((char)payload[i]);
	}
	Serial.println("");
	if (strcmp(topic, "ESP32_1/showOutTemp") == 0) {
		char* buffer = (char*)payload;
		buffer[length] = '\0';
		temperature = String(buffer);
	}
}

void reconnect() {
	// Loop until we're reconnected
	while (!client.connected()) {
		Serial.print("Attempting MQTT connection...");
		// Attempt to connect
		if (client.connect("ESP32_1")) {
			Serial.println("connected");
			// Once connected, publish an announcement...
			client.publish("ESP32_1/status", "ESP32_1 connected");
			// ... and resubscribe
			client.subscribe("ESP32_1/showOutTemp");
		}
		else {
			Serial.print("failed, rc=");
			Serial.print(client.state());
			Serial.println(" try again in 5 seconds");
			// Wait 5 seconds before retrying
			delay(5000);
		}
	}
}

void setup() {
	Serial.begin(115200);
	// Setup wifi
	setup_wifi();
	client.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
	client.setCallback(callback);
	// Initialising the UI will init the display too.
	display.init();
	display.flipScreenVertically();
	display.setFont(ArialMT_Plain_10);
}


void loop() {
	// process MQTT connection
	if (!client.connected()) {
		reconnect();
	}
	client.loop();
	// clear the display
	long now = millis();
	if (now - lastRefresh > refreshScreenPeriod) {
		lastRefresh = now;
		display.clear();
		showOutTemp();
	}
}

void showOutTemp() {
	display.setFont(ArialMT_Plain_10);
	display.setTextAlignment(TEXT_ALIGN_LEFT);
	display.drawString(20, 0, "OUT Temperature");
	display.setFont(ArialMT_Plain_24);
	display.drawString(30, 20, temperature + " C");
	// write the buffer to the display
	display.display();
}
