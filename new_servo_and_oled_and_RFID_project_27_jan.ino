#include <Servo.h> // Include the Servo library
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <MFRC522.h> // Include the RFID library

// Define pins for OLED
#define TFT_CS     10
#define TFT_RST    9
#define TFT_DC     8

// RFID Module pins
#define RST_PIN    5    // RST pin for RFID module
#define SS_PIN     6    // SDA (SS) pin for RFID module

MFRC522 rfid(SS_PIN, RST_PIN); // Create an RFID object

// Create display object
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// Define pin numbers for PIR sensor and LED/buzzer
const int pirSensorPin = 2;  // PIR sensor connected to pin 2
const int outputPin = 3;    // LED connected to pin 3
const int buzzerPin = 3;    // Buzzer connected to pin 3
const int servoPin = 7;      // Servo motor connected to pin 7

Servo myServo; // Create a Servo object

// Variables for PIR sensor
int pirState = LOW;          // Variable to hold the PIR sensor state
bool motionDetected = false; // Flag to indicate if motion was already handled

// Durations for LED, buzzer, and servo operation
const unsigned int buzzerLedDuration = 2000; // LED and buzzer on time (2 seconds)
const unsigned int servoDuration = 6000;    // Servo on time (6 seconds)

// Joystick Pins
const int VRx = A0;  // X-axis of joystick (not used in this code)
const int VRy = A1;  // Y-axis of joystick
const int SW = 4;    // Button (click, used for Lecturer Info)

const int totalScreens = 14; // 12 months + 2 extra screens (lecturer info and availability)
int currentScreen = 0;       // Start at Lecturer Info screen


const int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};


const char* daysOfWeek[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

// Variables for joystick navigation
int lastJoystickPosition = 512; 
int debounceDelay = 300;        
unsigned long lastDebounceTime = 0;


bool greenSlots[12][31]; // Available slots
bool redSlots[12][31];   // Booked slots

void setup() {
  // Initialize PIR sensor, LED, buzzer, and servo
  pinMode(pirSensorPin, INPUT);  // PIR sensor as input
  pinMode(outputPin, OUTPUT);    // LED as output
  pinMode(buzzerPin, OUTPUT);    // Buzzer as output
  myServo.attach(servoPin);      // Attach the servo to the defined pin

  // Initialize OLED display
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  tft.fillScreen(ST7735_BLACK);   // Black background
  tft.setTextColor(ST7735_WHITE); // Default text color
  pinMode(SW, INPUT_PULLUP);  // Initialize joystick button with pull-up resistor

  SPI.begin();         // Start SPI bus
  rfid.PCD_Init();     // Initialize RFID module

  generateRandomSlots();

  displayLecturerInfo();

  Serial.begin(9600);

  myServo.write(0); // Move servo to 0 degrees
}

void loop() {
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    handleRFIDTag();
    rfid.PICC_HaltA();       // Halt card
    rfid.PCD_StopCrypto1();  // Stop encryption
  }

  pirState = digitalRead(pirSensorPin);  // Read PIR sensor state

  if (pirState == HIGH && !motionDetected) { // Motion detected for the first time
    Serial.println("Motion detected!");

    digitalWrite(outputPin, HIGH);

    digitalWrite(buzzerPin, HIGH);

    myServo.write(90);

    delay(buzzerLedDuration);

    digitalWrite(outputPin, LOW);
    digitalWrite(buzzerPin, LOW);

    delay(servoDuration - buzzerLedDuration);

    myServo.write(0);

    motionDetected = true; // Mark motion as handled
  }

  if (pirState == LOW && motionDetected) { // Motion ends
    motionDetected = false; // Reset for the next motion event
  }

  int joystickValue = analogRead(VRy);

  if (millis() - lastDebounceTime > debounceDelay) {
    if (joystickValue > 600) { // Joystick moved down
      lastDebounceTime = millis();
      navigateScreens(1); // Next screen
    } else if (joystickValue < 400) { // Joystick moved up
      lastDebounceTime = millis();
      navigateScreens(-1); // Previous screen
    }
  }

  delay(50);
}

void handleRFIDTag() {
  Serial.print("Card UID: ");
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(rfid.uid.uidByte[i], HEX);
  }
  Serial.println();

  if (isValidCard(rfid.uid.uidByte, rfid.uid.size)) {
    Serial.println("Valid card detected! Unlocking...");
    digitalWrite(outputPin, HIGH);

    digitalWrite(buzzerPin, HIGH);

    myServo.write(90);

    delay(buzzerLedDuration);

    digitalWrite(outputPin, LOW);
    digitalWrite(buzzerPin, LOW);

    delay(servoDuration - buzzerLedDuration);

    myServo.write(0);
  } else {
    Serial.println("Invalid card.");
  }
}

bool isValidCard(byte *uid, byte size) {
  byte authorizedUID[] = {0x69, 0xDC, 0x29, 0x03};

  if (size != sizeof(authorizedUID)) return false;

  for (byte i = 0; i < size; i++) {
    if (uid[i] != authorizedUID[i]) return false;
  }
  return true;
}

void navigateScreens(int direction) {
  currentScreen = (currentScreen + direction + totalScreens) % totalScreens;

  if (currentScreen == 0) {
    displayLecturerInfo();
  } else if (currentScreen == 1) {
    displayPressButtonToCheckAvailability();
  } else {
    displayMonthCalendar(currentScreen - 2);
  }
}

void displayLecturerInfo() {
  tft.fillScreen(ST7735_BLACK);
  tft.setTextColor(ST7735_WHITE);
  tft.setTextSize(1);
  tft.setCursor(10, 30);
  tft.print("Lecturer: Jonas Poehler");

  tft.setCursor(10, 50);
  tft.print("Room Number: H-A 8114");

  tft.setCursor(10, 70);
  tft.print("University of Siegen");

  tft.setCursor(135, 100);
  tft.print("Next");
}

void displayPressButtonToCheckAvailability() {
  tft.fillScreen(ST7735_BLACK);
  tft.setTextColor(ST7735_WHITE);
  tft.setTextSize(1.5);
  tft.setCursor(20, 30);
  tft.print("LOOK FOR PROFESSOR");

tft.setCursor(40, 50);
  tft.print("AVAILBILITY");

  tft.setCursor(20, 70);
  tft.print("FROM AVAILABLE SLOTS");

  tft.setCursor(135, 100);
  tft.print("Next");
}

void displayMonthCalendar(int monthIndex) {
  tft.fillScreen(ST7735_BLACK);
  tft.setTextColor(ST7735_WHITE);
  tft.setTextSize(1);
  tft.setCursor(10, 10);
  tft.print(getMonthName(monthIndex));
  tft.setCursor(100, 10);
  tft.print("2025");

  int x = 5, y = 30;
  for (int i = 0; i < 7; i++) {
    tft.setCursor(x + (i * 20), y);
    tft.print(daysOfWeek[i]);
  }

  int day = 1;
  y += 15;
  int firstDay = calculateFirstDayOfMonth(monthIndex);

  for (int i = 0; i < 6; i++) {
    for (int j = 0; j < 7; j++) {
      if (i == 0 && j < firstDay) continue;
      if (day <= daysInMonth[monthIndex]) {
        tft.setCursor(x + (j * 20), y);

        if (j == 0 || j == 6) {
          tft.setTextColor(ST7735_WHITE);
        } else if (greenSlots[monthIndex][day - 1]) {
          tft.setTextColor(ST7735_GREEN);
        } else if (redSlots[monthIndex][day - 1]) {
          tft.setTextColor(ST7735_RED);
        } else {
          tft.setTextColor(ST7735_WHITE);
        }

        tft.print(day);
        day++;
      }
    }
    y += 15;
  }

  tft.setTextColor(ST7735_WHITE);
}

void generateRandomSlots() {
  for (int month = 0; month < 12; month++) {
    for (int day = 0; day < daysInMonth[month]; day++) {
      greenSlots[month][day] = random(0, 2);
      redSlots[month][day] = !greenSlots[month][day];
    }
  }
}

int calculateFirstDayOfMonth(int monthIndex) {
  int firstDay = 3;

  for (int i = 0; i < monthIndex; i++) {
    firstDay += daysInMonth[i];
  }

  return firstDay % 7;
}

const char* getMonthName(int monthIndex) {
  const char* monthNames[] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
  return monthNames[monthIndex];
}