#include <ESP8266WiFi.h>

// Networking details
const char* wifi_ssid = "your-ssid";
const char* wifi_password = "your-password";
const char* host = "influxdb-hostname";
const int port = 8086;
const char* influx_database = "telegraf";
const char* influx_username = "telegraf";
const char* influx_password = "password";
const char* influx_measurement = "temperature,host=esp-01,location=outdoors";
const char* influx_field = "value";

// Voltage divider details
const float R1 = 4.7;  // Fixed resistor next to thermistor. 0 if not required.
const float R2 = 2;  // Fixed resistor on other side of divider
const float Vcc = 3.3;  // Input voltage

// Get three measurements like below, then use
// http://www.thinksrs.com/downloads/programs/Therm%20Calc/NTCCalibrator/NTCcalculator.htm
//  23.7C = 4.45kohm
//   5.5C = 9.42kohm
// -17.9C = 34.26kohm
const float BETA = 3650;
const float NOM_RESIST = 3999;
const float NOM_TEMP = 25;

// Pin to measure from
const int analogInPin = A0;

void setup() {
  // Set up status LED
  pinMode(LED_BUILTIN, OUTPUT);

  // Open serial port
  Serial.begin(115200);
  delay(10);

  // Connect to WiFi
  Serial.println();
  Serial.print("Connecting to ");
  Serial.print(wifi_ssid);
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// Converts resistance (in ohms) to temperature (in deg C)
float resistanceToCelsius(float resistance) {
  // Simplified Steinhart-Hart
  float temp = log(resistance / NOM_RESIST) / BETA;
  temp += 1.0 / (NOM_TEMP + 273.15);
  temp = 1 / temp;
  temp -= 273.15; // convert to C
  return temp;
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(10000);
  digitalWrite(LED_BUILTIN, LOW);

  // Read the thermistor's voltage and thus get its resistance and temperature
  float voltage = analogRead(analogInPin);
  voltage = voltage / 1024;
  float totalResistance = ((Vcc * R2) / voltage) - R2;
  float resistance = (totalResistance - R1) * 1000;
  float temp = resistanceToCelsius(resistance);

  // Print status to serial console
  Serial.print("Sensor voltage: ");
  Serial.println(voltage);
  Serial.print("Resistance: ");
  Serial.println(resistance);
  Serial.print("Temp: ");
  Serial.println(temp);

  // Send data to InfluxDB
  Serial.print("Connecting to ");
  Serial.print(host);
  Serial.print(":");
  Serial.println(port);

  // Make a new client to connect to the server with
  WiFiClient client;
  if (!client.connect(host, port)) {
    Serial.println("connection failed");
    return;
  }

  // Build the request URI with the database, username and password
  String url = "/write?db=";
  url += influx_database;
  url += "&u=";
  url += influx_username;
  url += "&p=";
  url += influx_password;
  Serial.print("Requesting URL: ");
  Serial.println(url);

  // Make the POST body containing the data
  String body = String(influx_measurement) + " " + influx_field + "=" + temp;
  Serial.print("Body: ");
  Serial.println(body);

  // Build the full POST request
  client.println(String("POST ") + url + " HTTP/1.1");
  client.println(String("Host: ") + host);
  client.println("Connection: close");
  client.print("Content-Length: ");
  client.println(body.length());
  client.println();
  client.println(body);

  // Wait for the response
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println("Client timed out!");
      client.stop();
      return;
    }
  }

  // Read the reply and print it out
  Serial.println("Got response:");
  while (client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  Serial.println();
}
