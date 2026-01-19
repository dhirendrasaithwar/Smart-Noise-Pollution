#include <WiFiNINA.h>
#include "ThingSpeak.h"

// -------- WiFi Details --------
char ssid[] = "Badri";        // Your WiFi name
char pass[] = "badri1234";    // Your WiFi password

// -------- ThingSpeak Details --------
unsigned long channelID = 3193052;
const char * writeAPIKey = "1GUH73QE8T7XSRTT";

WiFiClient client;

// -------- Pins --------
int micPin = A0;       // KY-037 microphone analog output

// Traffic light pins
int redLED = 8;
int yellowLED = 9;
int greenLED = 10;

// -------- Thresholds (will be auto-calculated) --------
int lowThreshold;
int highThreshold;

void setup() {
  Serial.begin(9600);

  pinMode(redLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(greenLED, OUTPUT);

  // Connect to WiFi
  Serial.print("Connecting to WiFi");
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    Serial.print(".");
    delay(3000);
  }
  Serial.println("\nWiFi Connected!");

  ThingSpeak.begin(client);
  Serial.println("Smart Noise Pollution Monitor Started");

  // -------- Auto-calibrate thresholds --------
  Serial.println("Calibrating ambient noise...");
  int minNoise = 1023;
  int maxNoise = 0;

  for (int i = 0; i < 100; i++) {        // Sample 100 readings
    int val = analogRead(micPin);
    if (val < minNoise) minNoise = val;
    if (val > maxNoise) maxNoise = val;
    delay(50);
  }

  lowThreshold = minNoise + (maxNoise - minNoise) / 3;     // Green → Yellow
  highThreshold = minNoise + 2 * (maxNoise - minNoise) / 3; // Yellow → Red

  Serial.print("Calibration done! ");
  Serial.print("Low Threshold = "); Serial.print(lowThreshold);
  Serial.print(", High Threshold = "); Serial.println(highThreshold);
}

void loop() {
  int soundValue = analogRead(micPin);

  Serial.print("Sound Level: ");
  Serial.println(soundValue);

  // Turn OFF all LEDs
  digitalWrite(redLED, LOW);
  digitalWrite(yellowLED, LOW);
  digitalWrite(greenLED, LOW);

  // -------- Edge AI Decision Logic --------
  if (soundValue < lowThreshold) {
    digitalWrite(greenLED, HIGH);
    Serial.println("Green LED ON (Low noise)");
  }
  else if (soundValue < highThreshold) {
    digitalWrite(yellowLED, HIGH);
    Serial.println("Yellow LED ON (Medium noise)");
  }
  else {
    digitalWrite(redLED, HIGH);
    Serial.println("Red LED ON (High noise)");
  }

  // -------- Send Data to ThingSpeak --------
  ThingSpeak.setField(1, soundValue);
  int status = ThingSpeak.writeFields(channelID, writeAPIKey);

  if (status == 200) {
    Serial.println("Data uploaded to ThingSpeak");
  } else {
    Serial.print("ThingSpeak upload failed, error: ");
    Serial.println(status);
  }

  delay(15000); // ThingSpeak update interval
}
