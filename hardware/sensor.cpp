#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include <SPI.h>
#include <Wire.h>
#include <PubSubClient.h>
#include <WiFi.h>

// WiFi credentials
const char* ssid = "";
const char* password = "";

// MQTT server details
const char* mqtt_server = "";
const int mqtt_port = ;

// Define SPI pins
#define BME_CS 4     // Chip Select (CS)
#define BME_SCK 5     // SPI Clock (SCK)
#define BME_MOSI 19   // Master Out, Slave In (MOSI)
#define BME_MISO 21   // Master In, Slave Out (MISO)

WiFiClient espClient;
PubSubClient client(espClient);

// Create BME680 object
Adafruit_BME680 bme(BME_CS);

void setup() {
  Serial.begin(115200);
  Serial.println("Starter opp BME680...");

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.println("Kobler til WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi tilkoblet");

  // Set up MQTT server
  client.setServer(mqtt_server, mqtt_port);
  Serial.println("MQTT-server satt opp");

  // Initialize SPI with custom pins
  SPI.begin(BME_SCK, BME_MISO, BME_MOSI, BME_CS);

  // Initialize BME680 sensor
  if (!bme.begin()) {
    Serial.println("Kunne ikke finne en gyldig BME680-sensor, sjekk ledninger!");
    while (1);
  }
  Serial.println("BME680 initialisert!");

  // Configure BME680 settings
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 100);  // 100ms ved 320°C
}

void reconnect() {
  // Loop until reconnected
  while (!client.connected()) {
    Serial.print("Prøver å koble til MQTT-server...");
    if (client.connect("ESP32Client")) {
      Serial.println("Koblet til MQTT");
    } else {
      Serial.print("Feilet, rc=");
      Serial.print(client.state());
      delay(5000);
    }
  }
}

void loop() {
  // Ensure MQTT connection
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Perform a sensor reading
  if (bme.performReading()) {
    // Read sensor data
    float fuktighet = bme.humidity;
    float temperatur = bme.temperature;
    float gassmotstand = bme.gas_resistance;

    // Print sensor data to Serial Monitor
    Serial.print("Temperatur: ");
    Serial.print(temperatur);
    Serial.println(" °C");

    Serial.print("Fuktighet: ");
    Serial.print(fuktighet);
    Serial.println(" %");

    Serial.print("Trykk: ");
    Serial.print(bme.pressure / 100.0);
    Serial.println(" hPa");

    Serial.print("Gassmotstand: ");
    Serial.print(gassmotstand / 1000.0);
    Serial.println(" kOhm");

    // Publish data to MQTT topics
    char msg[50];
    snprintf(msg, 50, "%.2f", fuktighet);
    client.publish("sensor/fuktighet", msg);

    snprintf(msg, 50, "%.2f", temperatur);
    client.publish("sensor/temperatur", msg);

    snprintf(msg, 50, "%.2f", gassmotstand / 1000.0);
    client.publish("sensor/gassmotstand", msg);
  } else {
    Serial.println("Feil ved lesing av sensordata.");
  }

  delay(2000);
}