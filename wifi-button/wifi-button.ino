#include <WiFi.h>

// The bitmask and the button pins have to match!
#define BUTTON_PIN_BITMASK 0x300000000 // 2^32 | 2^33
#define BUTTON_1_PIN GPIO_NUM_32
#define BUTTON_2_PIN GPIO_NUM_33
#define LED_1_PIN GPIO_NUM_14
#define LED_2_PIN GPIO_NUM_15

const char* ssid     = "your_network_name_here";
const char* password = "network_password";

// These are set up for Belkin WeMo lights - you'd have to change the code
// below to activate other sorts of things (they'll need a HTTP API for that,
// or you can go via IFTTT's webhooks for anything it supports)
const char* host1 = "192.168.2.245";
const char* host2 = "192.168.2.239";
const int port1 = 49153;
const int port2 = 49152;

// Only used for debugging
RTC_DATA_ATTR int bootCount = 0;

void setup()
{
  // Set up button pins to read and read state straight away
  pinMode(BUTTON_1_PIN, INPUT);
  pinMode(BUTTON_2_PIN, INPUT);
  pinMode(LED_1_PIN, OUTPUT);
  pinMode(LED_2_PIN, OUTPUT);
  int button = 0;
  int button_pin = 0;
  const char* host;
  if (digitalRead(BUTTON_1_PIN) == 1) {
    button = 1;
    button_pin = LED_1_PIN;
    host = host1;
  } else if (digitalRead(BUTTON_2_PIN) == 1) {
    button = 2;
    button_pin = LED_2_PIN;
    host = host2;
  }

  // Turn on the light on the pressed button
  if (button_pin > 0) {
    digitalWrite(button_pin, HIGH);
  }

  // Open serial port
  Serial.begin(115200);

  // Turn on the built-in LED on boot
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));

  // Work out what mode we are in
  if (button > 0) {
    // Woken up by a button
    Serial.println("Woken up by button " + String(button));

    // Start WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
    }
    Serial.println("WiFi connected");
    Serial.println(WiFi.localIP());

    /*** Device specific section - replace this with whatever you want. ***/

    // Get the current switch state
    int state = get_switch_state(host, port1);
    Serial.println("\r\n");
    int port = port1;
    if (state >= 0) {
      Serial.println("Port 1 works, state is " + String(state));
    } else {
      state = get_switch_state(host, port2);
      if (state >= 0) {
        Serial.println("Port 2 works, state is " + String(state));
        port = port2;
      } else {
        Serial.println("No response on either port.");
      }
    }

    // Do the request
    if (state < 0) {
      error_flash(button_pin, 5);
    } else {
      Serial.println("Setting state to " + String(1 - state));
      if (set_switch_state(host, port, 1 - state)) {
        Serial.println("State set successfully.");
      } else {
        Serial.println("Failure setting state.");
        error_flash(button_pin, 7);
      }
    }

    /*** End device specific section ***/

  } else {
    Serial.println("Woken up without button");
  }

  // Set up wakeup
  Serial.println("Sleeping");
  esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);
  esp_deep_sleep_start();
}


/*
 * Flashes an LED in an error state.
 * */
void error_flash (int led_pin, int num) {
  for (int i = 0; i < num; i++) {
    digitalWrite(led_pin, HIGH);
    delay(100);
    digitalWrite(led_pin, LOW);
    delay(100);
  }
}


/*
 * Gets the current switch state. Returns -1 on error.
 * */
int get_switch_state(const char* host, int port) {
  WiFiClient client;
  // Try to open a TCP connection to the switch
  if (!client.connect(host, port)) {
      Serial.println("connection failed");
      return -1;
  }

  // Make and send the request
  bool success = upnp_request(client, "GetBinaryState", 1);
  if (!success) return -1;

  while(client.available()) {
    String line = client.readStringUntil('\r');
    //Serial.print(line);
    int result_index = line.indexOf("<BinaryState>");
    if (result_index > 0) {
      return line.substring(result_index + 13, result_index + 14).toInt();
    }
  }
  return -1;
}


/*
 * Turns a switch on or off.
 * */
bool set_switch_state(const char* host, int port, int state) {
  WiFiClient client;

  // Try to open a TCP connection to the switch
  if (!client.connect(host, port)) {
      Serial.println("connection failed");
      return false;
  }

  // Make and send the request
  bool success = upnp_request(client, "SetBinaryState", state);
  if (!success) return false;

  //while(client.available()) {
  //  String line = client.readStringUntil('\r');
  //  Serial.print(line);
  //}
  //Serial.println("\r\n");
  return true;
}

/*
 * Sends a uPNP request to a client
 * */
bool upnp_request(WiFiClient client, char* action, int value) {
  String url = "/upnp/control/basicevent1";
  String body = String("<?xml version=\"1.0\" encoding=\"utf-8\"?>") +
    "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" " +
    "s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">" +
    "<s:Body><u:" + action + " xmlns:u=\"urn:Belkin:service:basicevent:1\">" +
    "<BinaryState>" + value + "</BinaryState></u:" + action + "></s:Body></s:Envelope>";
  client.print(
    String("POST ") + url + " HTTP/1.0\r\n" +
    "Content-type: text/xml; charset=\"utf-8\"\r\n" +
    "SOAPACTION: \"urn:Belkin:service:basicevent:1#" + action + "\"\r\n" +
    "Content-Length: " + body.length() + "\r\n" +
    "\r\n" + body
  );
  // Check request timeout
  unsigned long timeout = millis();
  while (client.available() == 0) {
      if (millis() - timeout > 1000) {
          Serial.println(">>> Client Timeout !");
          client.stop();
          return false;
      }
  }
  return true;
}


void loop()
{
  // Never called
}
