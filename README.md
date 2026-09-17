# Smart Silent Help Request System

An ESP32-based IoT safety and assistance system developed as a university group project.

The system uses two ESP32 devices connected through Firebase Realtime Database. Device A collects input from a push button and PIR motion sensor, while Device B reads the incoming states and automatically controls visual and audible alerts.

The project also includes a browser-based monitoring dashboard hosted by Device B.

## Project Overview

The Smart Silent Help Request System was designed to provide a simple silent alert mechanism using motion detection and a physical help-request button.

The system combines:

- Real-time sensor monitoring
- Device-to-device communication
- Automated alert logic
- Cloud-based state synchronization
- Web-based monitoring and remote control

System Logic
Button	Motion	System Response
0	0	No alert
1	0	Green LED
0	1	Green LED
1	1	Red LED + Buzzer

The green LED represents a caution state when either motion or the help button is detected.

The red LED and buzzer are activated only when both motion and the help button are detected.

Device A

Device A functions as the sensing unit.

It:

Reads the push-button state
Reads the PIR motion sensor
Sends both values to Firebase Realtime Database
Updates the cloud state continuously
Hardware
ESP32
PIR Motion Sensor
Push Button
Device B

Device B functions as the actuation and monitoring unit.

It:

Reads Device A data from Firebase
Applies the system decision logic
Controls the green LED
Controls the red LED
Activates the buzzer during a full alert
Hosts the local monitoring webpage
Provides remote reset functionality
Hardware
ESP32
Red LED
Green LED
Buzzer
Resistors
Web Dashboard

The ESP32 hosts a browser-based dashboard that provides real-time system monitoring.

The dashboard displays:

Push-button status
PIR motion status
Green LED status
Red LED status
Buzzer status
Alert state
Device reset control
Automatically refreshed system information
Technologies
ESP32
Arduino / Embedded C++
Firebase Realtime Database
Wi-Fi
HTML
CSS
JavaScript
Embedded Web Server
PIR Motion Sensor
Push Button
LEDs
Buzzer
Project Structure
smart-silent-help-request-system/
├── README.md
├── .gitignore
├── device-a/
│   └── device_a.ino
└── device-b/
    └── device_b.ino
How It Works
Device A monitors the PIR sensor and push button.
Sensor states are uploaded to Firebase Realtime Database.
Device B retrieves the current states.
Device B evaluates the combination of inputs.
The appropriate alert state is activated.
The web dashboard displays the current system status.
A remote reset function can be used to reset Device B.
Key Learning Outcomes

This project provided practical experience in:

IoT system development
ESP32 programming
Sensor integration
Firebase Realtime Database
Real-time device communication
Embedded decision logic
Web dashboard development
Hardware and software troubleshooting
Team-based system development
Academic Context

This project was developed as a university group project for an Internet of Things course.

It demonstrates the integration of sensing, cloud communication, embedded actuation, and browser-based monitoring within a two-device IoT architecture.

Security Note

Wi-Fi and Firebase credentials in the public repository are represented using placeholders.

Real credentials should never be committed to a public repository.

Future Improvements

Possible future improvements include:

Mobile notifications
User authentication
Event history and logging
Cloud-hosted monitoring dashboard
Improved enclosure and hardware integration
Additional sensing devices

## System Architecture

```text
Push Button + PIR Sensor
          ↓
       Device A
        ESP32
          ↓
 Firebase Realtime Database
          ↓
       Device B
        ESP32
          ↓
  Alert Decision Logic
     ↓       ↓       ↓
 Green LED Red LED  Buzzer
          ↓
     Web Dashboard

