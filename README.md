# Knock-pattern-authentication-system
Microcontroller-based knock pattern authentication system designed for secure and discreet access control

# Description
This project implements a microcontroller-based authentication system that uses knock patterns as a secure access mechanism. 

The system detects vibration signals using a piezoelectric sensor, processes them through an amplifier, and compares the input pattern with a stored pattern to grant or deny access.

# Features
- Knock-based authentication (no keypad required)
- Adjustable sensitivity using potentiometer
- Real-time feedback via OLED display
- Servo-based locking mechanism
- Buzzer alerts for access granted/denied
- Noise filtering and debounce handling

# Working Principle
The piezoelectric sensor detects vibrations caused by knocks. These signals are amplified and read by the microcontroller.

The system records the timing between knocks and compares it with a predefined pattern. If the pattern matches within a tolerance range, access is granted; otherwise, it is denied.

# Components Used
- Arduino Uno
- Piezoelectric sensor
- LM358 Op-Amp (for amplification)
- OLED display (SPI)
- Servo motor
- Buzzer
- Push button
- Potentiometer
- Breadboard & wires

# Connections
- Arduino Uno
- Piezoelectric sensor
- LM358 Op-Amp (for amplification)
- OLED display (SPI)
- Servo motor
- Buzzer
- Push button
- Potentiometer
- Breadboard & wires

# How To Use
1. Power the system
2. Default state shows "Knock Lock Ready"
3. Knock the correct pattern to unlock
4. Press button to enter recording mode
5. Set a new knock pattern

# Future Improvements
- IoT integration (remote monitoring)
- EEPROM storage for permanent pattern saving
- Multi-user authentication
- Mobile app control

# Author
Karthikeyan I
