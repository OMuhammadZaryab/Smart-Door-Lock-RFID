# Door Lock and RFID-Based Security System

## Overview
This project is an ESP32-based smart door lock system that uses RFID authentication, a servo motor for door control, an OLED display for status updates, and a PIR sensor for motion detection. The system integrates with a Flask server to log door events and send email notifications.

## Features
- RFID-based authentication
- Motion detection with a PIR sensor
- Joystick-controlled OLED display for lecturer information and calendar navigation
- Wi-Fi-enabled door status updates
- Flask server with a web-based dashboard
- Email notifications on door status changes

## Hardware Requirements
- ESP32 Development Board
- RFID Module (MFRC522)
- Servo Motor
- PIR Sensor
- Joystick Module
- OLED Display (Adafruit ST7735)
- Buzzer and LED

## Software Requirements
- Arduino IDE with ESP32 libraries
- Flask (for the server backend)
- Wi-Fi connection
- Python 3 (for running the Flask server)

## Installation & Setup
### ESP32 Setup
1. Install the required libraries:
   ```cpp
   #include <WiFi.h>
   #include <HTTPClient.h>
   #include <Servo.h>
   #include <Adafruit_GFX.h>
   #include <Adafruit_ST7735.h>
   #include <SPI.h>
   #include <MFRC522.h>
   ```
2. Update Wi-Fi credentials in `doorlock_ppt.txt`:
   ```cpp
   const char* WIFI_SSID = "Your_SSID";
   const char* WIFI_PASSWORD = "Your_Password";
   ```
3. Upload the ESP32 sketch to your board.

### Flask Server Setup
1. Install Python dependencies:
   ```bash
   pip install flask
   ```
2. Run the Flask server:
   ```bash
   python flask_server.py
   ```
3. Access the web dashboard at `http://<server-ip>:5000/api/door_logs`.

## Usage
- Scan an RFID card to unlock the door.
- View the lecturer’s schedule using the joystick and OLED display.
- Monitor door status remotely via the Flask server.
- Receive email alerts when the door state changes.

## Future Improvements
- Add database support for logging access history.
- Implement real-time mobile notifications.
- Integrate with a mobile app for remote control.

## License
This project is open-source and available for use.
