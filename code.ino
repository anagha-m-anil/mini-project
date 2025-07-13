#include <SoftwareSerial.h>

// Pin definitions for SIM, GPS, and LED
#define SIM_TX 3
#define SIM_RX 2
#define GPS_TX 8
#define GPS_RX 9
#define X_PIN A0
#define Y_PIN A1
#define Z_PIN A2
#define LED_PIN 13  // Pin for LED (built-in LED on most boards)

// Pin definitions for motor control, alcohol sensor, IR sensor, and buzzer
const int motor1Pin1 = 4;  // IN1 of L298 (Motor 1)
const int motor2Pin1 = 5;  // IN3 of L298 (Motor 2)
const int alcoholPin = A3; // Analog input from MQ3 sensor
const int IRSensor = 6;    // Digital input from IR Eye Blink sensor
const int BUZZER = 7;      // Pin connected to the buzzer

// Threshold values
float threshold = 2.2;  // Accident detection threshold (adjustable)
const int alcoholThreshold = 400; // Alcohol detection threshold

// Create SoftwareSerial objects for SIM and GPS
SoftwareSerial sim800l(SIM_TX, SIM_RX);
SoftwareSerial gpsSerial(GPS_TX, GPS_RX);

float x, y, z;
String latitude = "", longitude = "";

void setup() {
  Serial.begin(9600);
  sim800l.begin(9600);
  gpsSerial.begin(9600);

  // Set motor pins as outputs
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor2Pin1, OUTPUT);

  // Set IR sensor pin as input
  pinMode(IRSensor, INPUT);

  // Set buzzer pin as output
  pinMode(BUZZER, OUTPUT);

  // Set LED pin as output
  pinMode(LED_PIN, OUTPUT);

  delay(3000);
  Serial.println("System Ready...");
}

void loop() {
  // Read ADXL335 sensor
  x = analogRead(X_PIN) * (5.0 / 1023.0);
  y = analogRead(Y_PIN) * (5.0 / 1023.0);
  z = analogRead(Z_PIN) * (5.0 / 1023.0);

  Serial.print("X: "); Serial.print(x);
  Serial.print(" Y: "); Serial.print(y);
  Serial.print(" Z: "); Serial.println(z);

  // Detect accident based on threshold
  if (x > threshold || y > threshold || z > threshold) {
    Serial.println("Accident Detected!");
    
    // Turn on LED
    digitalWrite(LED_PIN, HIGH);

    // Get GPS location
    getGPSLocation();
    
    // Send alert
    sendSMS();
    
    delay(10000); // Avoid multiple triggers in a short time

    // Turn off LED after delay
    digitalWrite(LED_PIN, LOW);
  }

  // Alcohol detection logic
  int alcoholValue = analogRead(alcoholPin);
  Serial.print("Alcohol Value: ");
  Serial.println(alcoholValue);

  if (alcoholValue > alcoholThreshold) {
    // Stop the motors if alcohol is detected
    stopMotors();
    Serial.println("Alcohol detected! Motors stopped.");
  } else {
    // Start the motors if no alcohol is detected
    startMotors();
    Serial.println("No alcohol detected. Motors running.");
  }

  // Read IR eye blink sensor
  int statusSensor = digitalRead(IRSensor);

  if (statusSensor == HIGH) {
    // If no blink detected, turn off the buzzer
    digitalWrite(BUZZER, HIGH);
  } else {
    // If blink detected, turn on the buzzer
    digitalWrite(BUZZER, LOW);
  }

  delay(500); // Small delay to avoid rapid switching
}

// Function to get GPS location
void getGPSLocation() {
  Serial.println("Fetching GPS location...");
  while (gpsSerial.available()) {
    char c = gpsSerial.read();
    if (c == '$') {
      String nmeaData = gpsSerial.readStringUntil('\n');
      if (nmeaData.startsWith("GPGGA")) { // Standard GPS format
        parseGPS(nmeaData);
        break;
      }
    }
  }
}

// Function to parse GPS data
void parseGPS(String nmeaData) {
  char *ptr = strtok((char *)nmeaData.c_str(), ",");
  int field = 0;
  while (ptr) {
    field++;
    if (field == 3) latitude = String(ptr);
    if (field == 5) longitude = String(ptr);
    ptr = strtok(NULL, ",");
  }
  Serial.print("Latitude: "); Serial.println(latitude);
  Serial.print("Longitude: "); Serial.println(longitude);
}

// Function to send SMS alert
void sendSMS() {
  String smsMessage = "Accident Alert! Location: https://maps.google.com/?q=" + latitude + "," + longitude;
  
  Serial.println("Sending SMS...");
  sim800l.println("AT+CMGF=1"); 
  delay(1000);
  sim800l.println("AT+CMGS=\"+919961760844\"");  // Replace with recipient number
  delay(1000);
  sim800l.println(smsMessage);
  delay(1000);
  sim800l.write(26); // End SMS
  Serial.println("SMS Sent.");
}

// Function to start both motors
void startMotors() {
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor2Pin1, HIGH);
}

// Function to stop both motors
void stopMotors() {
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor2Pin1, LOW);
}
