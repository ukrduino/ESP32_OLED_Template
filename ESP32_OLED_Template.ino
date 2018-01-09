// alias for `#include "SSD1306Wire.h"`
#include <DHTesp.h>
#include "SSD1306.h"
#include "Credentials.h"
#include "WiFi.h"
#include "Icons.h"

// Initialize the OLED display using Wire library
SSD1306  display(0x3c, 5, 4);
DHTesp dht;

/** Pin number for DHT22 data pin */
int dhtPin = 15;


#include <PubSubClient.h>
// Update these with values suitable for your network.

const char* mqtt_server = MQTT_SERVER_IP;
WiFiClient espClient;
PubSubClient client(espClient);
String outTemperature = "0";
String sensorTemperature = "0";
String sensorHumidity = "0";
String iconId = "";
float h = 0;
float t = 0;
unsigned long lastGetSensorData = 0;
int getSensorDataPeriod = 15000;
int currentScreen = 0;
int nextScreen = 1;
unsigned long lastScreenChange = 0;
int screenDisplayPeriod = 5000;

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
		outTemperature = String(buffer);
	}
	if (strcmp(topic, "ESP32_1/showIcon") == 0) {
		char* buffer = (char*)payload;
		buffer[length] = '\0';
		iconId = String(buffer);
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
			client.subscribe("ESP32_1/showIcon");
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
	display.setFont(ArialMT_Plain_10);
	dht.setup(dhtPin);
}


void loop() {
	// process MQTT connection
	if (!client.connected()) {
		reconnect();
	}
	client.loop();
	getDTHSensorData();
	processDisplay();
}

void getDTHSensorData() {
	long now = millis();
	if (now - lastGetSensorData > getSensorDataPeriod) {
		lastGetSensorData = now;
		// Reading temperature or humidity takes about 250 milliseconds!
		// Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
		float sensorHum = dht.getHumidity();
		// Read temperature as Celsius (the default)
		float sensorTemp = dht.getTemperature();
		// Read temperature as Fahrenheit (isFahrenheit = true)
		// Check if any reads failed and exit early (to try again).
		if (isnan(h) || isnan(t)) {
			Serial.println("Failed to read from DHT sensor!");
			return;
		}
		//Serial.print("Humidity: ");
		//Serial.print(h);
		//Serial.print(" %\t");
		//Serial.print("Temperature: ");
		//Serial.print(t);
		//Serial.println(" *C ");
		if (sensorHum != h || sensorTemp != t)
		{
			Serial.println(h);
			Serial.println(t);
			Serial.println(sensorHum);
			Serial.println(sensorTemp);
			h = sensorHum;
			t = sensorTemp;
			sensorTemperature = String(t);
			sensorHumidity = String(h);
			publishSensorData();
		}
	}
}

void publishSensorData() {
	client.publish("ESP32_1/temperature", String(sensorTemperature).c_str());
	client.publish("ESP32_1/humidity", String(sensorHumidity).c_str());
}

void processDisplay() {
	long now = millis();
	if (now - lastScreenChange > screenDisplayPeriod) {
		lastScreenChange = now;
		switch (nextScreen) {
		case 0:
			//Serial.print(outTemperature);
			nextScreen = 1;
			showOutTemp();
			break;
		case 1:
			//Serial.print(sensorTemperature);
			nextScreen = 2;
			showInTemp();
			break;
		case 2:
			//Serial.print(sensorHumidity);
			nextScreen = 3;
			showInHumidity();
			break;
		case 3:
			Serial.print(iconId);
			nextScreen = 0;
			showIcon();
			break;
		default:
			// statements
			break;
		}
	}
}


void showOutTemp() {
	display.clear();
	display.drawXbm(0, 13, out_temp_width, out_temp_height, out_temp_bits);
	display.setTextAlignment(TEXT_ALIGN_LEFT);
	display.setFont(Monospaced_plain_32);
	display.drawString(35, 10, outTemperature);
	// write the buffer to the display
	display.display();
}

void showInTemp() {
	display.clear();
	display.drawXbm(0, 13, in_temp_width, in_temp_height, in_temp_bits);
	display.setTextAlignment(TEXT_ALIGN_LEFT);
	display.setFont(Monospaced_plain_32);
	display.drawString(35, 10, String(t, 1));
	// write the buffer to the display
	display.display();
}

void showInHumidity() {
	display.clear();
	display.drawXbm(0, 13, drop_width, drop_height, drop_bits);
	display.setTextAlignment(TEXT_ALIGN_LEFT);
	display.setFont(Monospaced_plain_32);
	String hum = String(h, 0) + "%";
	display.drawString(35, 10, hum);
	// write the buffer to the display
	display.display();
}

void showIcon() {
	display.clear();
	if (iconId == "01")
	{
		display.drawXbm(30, 0, icon_height, icon_height, icon_01);
	}
	if (iconId == "02")
	{
		display.drawXbm(30, 0, icon_height, icon_height, icon_02);
	}
	if (iconId == "03")
	{
		display.drawXbm(30, 0, icon_height, icon_height, icon_03);
	}	
	if (iconId == "04")
	{
		display.drawXbm(30, 0, icon_height, icon_height, icon_04);
	}
	if (iconId == "09")
	{
		display.drawXbm(30, 0, icon_height, icon_height, icon_09);
	}
	if (iconId == "10")
	{
		display.drawXbm(30, 0, icon_height, icon_height, icon_10);
	}
	if (iconId == "11")
	{
		display.drawXbm(30, 0, icon_height, icon_height, icon_11);
	}
	if (iconId == "13")
	{
		display.drawXbm(30, 0, icon_height, icon_height, icon_13);
	}
	// write the buffer to the display
	display.display();
}