// This #include statement was automatically added by the Particle IDE.
#include <google-maps-device-locator.h>

// This #include statement was automatically added by the Particle IDE.
#include <TinyGPS++.h>

static const uint32_t GPSBaud = 9600;
static const int blue_led = D7;
static const double movingDelta = 0.0001;
int stationaryDelay = 30 * 60 * 1000;
int movingDelay = 60 * 1000;


// Init GPS library
TinyGPSPlus gps;
TinyGPSCustom antennaStatus(gps, "PGTOP", 2);

// Init cell location library
GoogleMapsDeviceLocator locator;

// Tracking where we last were
double lastLat = 0;
double lastLng = 0;
float lastSend = 0 - movingDelay;


void setup() {
    // Start debugging serial
    Serial.begin(115200);
    Serial.println("Booting");
    
    // Start GPS serial
    Serial1.begin(9600);
    Serial1.println("$PGCMD,33,1*6C");
    
    // On-demand location
    Particle.function("locateCell", locateCell);
    Particle.function("locateGps", locateGps);
    Particle.function("setMovingDelay", setMovingDelay);
    Particle.variable("movingDelay", movingDelay);

    // Setup debugging LED
    pinMode(blue_led, OUTPUT);
}


void loop() {
    
    // Debug with satellite info always
    Serial.print("Satellites: ");
    Serial.print(gps.satellites.value());
    Serial.print("  Antenna: ");
    Serial.print(antennaStatus.value());
    Serial.print("  Chars: ");
    Serial.print(gps.charsProcessed());
    Serial.print("  Failed: ");
    Serial.println(gps.failedChecksum());
    
    // See if we need to send
    bool needsSend = false;
    double latDiff = lastLat - gps.location.lat();
    double lngDiff = lastLng - gps.location.lng();
    latDiff = abs(latDiff);
    lngDiff = abs(lngDiff);
    float sentAge = millis() - lastSend;
    if (sentAge < 0) lastSend = 0;
    if (sentAge > stationaryDelay) {
        needsSend = true;
        Serial.print("Sending due to stationary delay ");
        Serial.println(sentAge);
    }
    if (sentAge > movingDelay && gps.location.isValid()) {
        if ((latDiff + lngDiff) > movingDelta) {
            needsSend = true;
            Serial.print("Sending due to movement ");
            Serial.println(sentAge);
        }
        if (gps.speed.knots() > 10) {
            needsSend = true;
            Serial.print("Sending due to speed ");
            Serial.println(sentAge);
        }
    }
    if (needsSend) {
        sendLocation();
    }
    
    // Check good GPS wiring
    if (millis() > 5000 && gps.charsProcessed() < 10)
    Serial.println(F("No GPS data received: check wiring"));
    
    smartDelay(5000);
    locator.loop();
    
}


// Sends the current location
void sendLocation() {
    // Send GPS location if valid
    if (gps.location.isValid()) {
        digitalWrite(blue_led, HIGH);
        
        String data = String::format(
          "{ \"lat\": %.6f, \"lng\": %.6f, \"alt\": %.0f, \"hdop\": %li, \"time\": \"%04u-%02u-%02uT%02u:%02u:%02uZ\", \"spd\": %lf, \"crs\": %lf, \"sat\": %lu }",
          gps.location.lat(),
          gps.location.lng(),
          gps.altitude.meters(),
          gps.hdop.value(),
          gps.date.year(),
          gps.date.month(),
          gps.date.day(),
          gps.time.hour(),
          gps.time.minute(),
          gps.time.second(),
          gps.speed.knots(),
          gps.course.deg(),
          gps.satellites.value()
        );
        Particle.publish("sendlocation", data, PRIVATE);
        Serial.print("Sent location lat: ");
        Serial.print(gps.location.lat(), 6);
        Serial.print(" long: ");
        Serial.println(gps.location.lng(), 6);
        digitalWrite(blue_led, LOW);
        // Store what we sent to see when we need to next
        lastSend = millis();
        lastLat = gps.location.lat();
        lastLng = gps.location.lng();
    } else {
        digitalWrite(blue_led, HIGH);
        String data = String::format(
          "{ \"time\": \"%04u-%02u-%02uT%02u:%02u:%02uZ\", \"sat\": %i }",
          gps.date.year(),
          gps.date.month(),
          gps.date.day(),
          gps.time.hour(),
          gps.time.minute(),
          gps.time.second(),
          gps.satellites.value()
        );
        Particle.publish("sendlocation", data, PRIVATE);
        Serial.println("Sent invalid location");
        digitalWrite(blue_led, LOW);
        // Store what we sent to see when we need to next
        lastSend = millis();
    }
}


// This custom version of delay() ensures that the gps object is being "fed".
static void smartDelay(unsigned long ms) {
  unsigned long start = millis();
  do {
    while (Serial1.available()) {
        byte c = Serial1.read();
        gps.encode(c);
        //Serial.print(char(c));
    }
  } while (millis() - start < ms);
}


// On-demand cell location
int locateCell(String extra) {
    locator.publishLocation();
    return 0;
}


// On-demand GPS location
int locateGps(String extra) {
    sendLocation();
    return 0;
}


// Dynamic setting of move delay
int setMovingDelay(String extra) {
    movingDelay = extra.toInt();
    return 0;
}
