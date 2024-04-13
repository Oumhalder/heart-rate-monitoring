
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// Define the pulse sensor settings
const int pulsePin = 32; // the pulse sensor pin
const int ledPin = 33; // the LED pin
int pulseValue; // the pulse sensor value
int bpm; // the heart rate in beats per minute

// Your WiFi credentialilan.
// Set password to "" for open networks.
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "****";
char pass[] = "****";
BlynkTimer timer;

void setup()
 {
  // Start the serial communication
  Serial.begin(9600);

  // Connect to WiFi
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Connect to Blynk
  Blynk.begin(auth, ssid, pass);
  while (!Blynk.connected()) {
    Serial.println("Connecting to Blynk...");
    delay(1000);
  }
  Serial.println("Connected to Blynk");

  // Set up the pulse sensor
  pinMode(pulsePin, INPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
}

void loop()
 {
  // Read the pulse sensor value
  pulseValue = analogRead(pulsePin);

  // Detect the pulse
  if (pulseValue > 600) {
    digitalWrite(ledPin, HIGH); // turn on the LED
    delay(100); // wait for a short time
    digitalWrite(ledPin, LOW); // turn off the LED
    bpm = 60000 / pulseValue; // calculate the heart rate in beats per minute
    Serial.print("Heart rate: ");
    Serial.print(bpm);
    Serial.println(" BPM");

    // Send the heart rate to Blynk
    Blynk.virtualWrite(V4, bpm);
    delay(200);

    // Print the heart rate on the serial monitor
    String message = "Heart rate: " + String(bpm) + " BPM";
    Serial.println(message);
  }

  // Run the Blynk loop
  Blynk.run();
}
