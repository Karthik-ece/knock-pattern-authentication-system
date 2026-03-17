#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_DC 9
#define OLED_CS 8
#define OLED_RESET 10

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, OLED_DC, OLED_RESET, OLED_CS);
Servo lockServo;

// Pins
int piezoPin = A0;
int potPin = A3;
int buzzer = 7;
int buttonPin = 2;   // button, one pin to GND
int servoPin = 6;

// Pattern variables
const int maxKnocks = 10;
float secretPattern[maxKnocks] = {1.0, 3.0, 5.0, 0,0,0,0,0,0,0}; // default example
float inputPattern[maxKnocks];

int knockThreshold = 10000;     // higher = stronger tap needed
int releaseThreshold = 150;   // must fall below to reset knockState
unsigned long lastKnockTime = 0;
int debounceTime = 1000;      // 1 second lockout per knock
bool knockState = false;      // avoid multiple counts from one knock

bool lastButtonState = HIGH;

// ----- STATE MACHINE -----
enum LockState { READY, LISTENING, RECORDING, NEW_PATTERN };
LockState currentState = READY;

// ----- SETUP -----
void setup() {
  Serial.begin(9600);

  pinMode(buzzer, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  lockServo.attach(servoPin);
  lockServo.write(0);

  display.begin(SSD1306_SWITCHCAPVCC);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  displayKnockReady();

  lastButtonState = digitalRead(buttonPin);
}

// ----- LOOP -----
void loop() {
  int potVal = analogRead(potPin);
  knockThreshold = map(potVal, 0, 1023, 160, 300); // optional sensitivity

  // ----- BUTTON EDGE DETECTION -----
  bool currentButtonState = digitalRead(buttonPin);
  if(lastButtonState == HIGH && currentButtonState == LOW) {
      if(currentState == READY) {
          // start recording
          currentState = RECORDING;
          displayRecording();
          recordKnockPattern();
          currentState = NEW_PATTERN;
          displayNewPattern();
      } else if(currentState == NEW_PATTERN) {
          // return to READY after new pattern set
          currentState = READY;
          displayKnockReady();
      }
  }
  lastButtonState = currentButtonState;

  int sensorValue = analogRead(piezoPin);

  // ----- KNOCK DETECTION -----
  if(currentState == READY) {
      if(sensorValue > knockThreshold && !knockState && (millis() - lastKnockTime) > debounceTime) {
          Serial.println("Knock detected!");
          knockState = true;
          lastKnockTime = millis();
          currentState = LISTENING;
          listenToKnock();    // 10-second listening
          currentState = READY; // return to default
      }
  }

  if(sensorValue < releaseThreshold && knockState) knockState = false;
}

// ----- DISPLAY FUNCTIONS -----
void displayKnockReady() {
  display.clearDisplay();
  display.setCursor(0,20);
  display.println("Knock Lock Ready");
  display.display();
}

void displayRecording() {
  display.clearDisplay();
  display.setCursor(0,20);
  display.println("Recording Mode");
  display.display();
}

void displayNewPattern() {
  display.clearDisplay();
  display.setCursor(0,20);
  display.println("New Pattern Set");
  display.display();
}

// ----- LISTEN FOR KNOCK PATTERN -----
void listenToKnock() {
  unsigned long startTime = millis();
  int knockCount = 0;

  while(millis() - startTime < 10000 && knockCount < maxKnocks) {  // 10s listening
      int sensorValue = analogRead(piezoPin);
      unsigned long elapsed = millis() - startTime;
      int seconds = elapsed / 1000;

      // Update OLED
      display.clearDisplay();
      display.setCursor(0,10);
      display.println("Listening Mode");
      display.setCursor(0,30);
      display.print("Time: ");
      display.print(seconds);
      display.print(" s");
      display.setCursor(0,50);
      display.print("Knocks: ");
      display.println(knockCount);
      display.display();

      // Detect knock
      if(sensorValue > knockThreshold && !knockState && (millis() - lastKnockTime) > debounceTime) {
          float knockTime = (millis() - startTime) / 1000.0; // seconds
          inputPattern[knockCount] = knockTime;
          knockCount++;
          Serial.print("Knock at ");
          Serial.print(knockTime);
          Serial.println(" seconds");
          tone(buzzer, 2000, 100);
          knockState = true;
          lastKnockTime = millis();
      }

      if(sensorValue < releaseThreshold && knockState) knockState = false;
  }

  // ----- VALIDATE PATTERN -----
  if(validatePattern(knockCount)) {
      display.clearDisplay();
      display.setCursor(0,20);
      display.println("Access Granted");
      display.display();
      tone(buzzer, 1500, 200);
      unlockDoor();
  } else {
      display.clearDisplay();
      display.setCursor(0,20);
      display.println("Wrong Pattern");
      display.display();
      for(int i=0;i<3;i++){
          tone(buzzer, 500, 150);
          delay(200);
      }
      delay(1000);
      displayKnockReady();
  }
}

// ----- VALIDATE KNOCK PATTERN -----
bool validatePattern(int knocks) {
  float tolerance = 0.5;  // seconds
  for(int i=0;i<knocks;i++){
      if(abs(secretPattern[i]-inputPattern[i]) > tolerance) return false;
  }
  return true;
}

// ----- RECORD NEW PATTERN -----
void recordKnockPattern() {
  unsigned long startTime = millis();
  int knockCount = 0;

  while(knockCount < maxKnocks) {
      int sensorValue = analogRead(piezoPin);
      unsigned long elapsed = millis() - startTime;
      int seconds = elapsed / 1000;

      // OLED update
      display.clearDisplay();
      display.setCursor(0,10);
      display.println("Recording Mode");
      display.setCursor(0,30);
      display.print("Time: ");
      display.print(seconds);
      display.print(" s");
      display.setCursor(0,50);
      display.print("Knocks: ");
      display.println(knockCount);
      display.display();

      if(sensorValue > knockThreshold && !knockState && (millis() - lastKnockTime) > debounceTime) {
          float knockTime = (millis() - startTime) / 1000.0;
          secretPattern[knockCount] = knockTime;
          knockCount++;
          Serial.print("Recorded knock at ");
          Serial.print(knockTime);
          Serial.println(" seconds");
          tone(buzzer, 2000, 100);
          knockState = true;
          lastKnockTime = millis();
      }

      if(sensorValue < releaseThreshold && knockState) knockState = false;

      // Stop after 10s max or 10s inactivity
      if(seconds >= 10) break;
      if(millis() - lastKnockTime > 10000 && knockCount > 0) break;
  }
}

// ----- SERVO CONTROL -----
void unlockDoor() {
  lockServo.write(90);
  delay(5000);
  lockServo.write(0);
  displayKnockReady();
}