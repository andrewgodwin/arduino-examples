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

const int analogInPin = A0;

void setup() {
  Serial.begin(115200);
}

float resistanceToCelsius(float resistance) {
  // Simplified Steinhart-Hart
  float temp = log(resistance / NOM_RESIST) / BETA;
  temp += 1.0 / (NOM_TEMP + 273.15);
  temp = 1 / temp;
  temp -= 273.15; // convert to C
  return temp;
}

void loop() {
  delay(2000);

  // Read the thermistor's voltage and thus get its resistance
  float voltage = analogRead(analogInPin);
  voltage = voltage / 1024;
  float totalResistance = ((Vcc * R2) / voltage) - R2;
  float resistance = (totalResistance - R1) * 1000;

  Serial.print("sensor voltage = ");
  Serial.println(voltage);

  Serial.print("resistance = ");
  Serial.println(resistance);

  Serial.print("temp = ");
  Serial.println(resistanceToCelsius(resistance));

}
