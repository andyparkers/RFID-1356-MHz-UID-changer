# RFID-1356-MHz-UID-changer
This device allows you to read, copy and write your own UID of MIFARE Classic cards to suitable RFID cards which emulate all the functions of original MIFARE Classic. This might be useful for pentesting ACS (access control systems) which rely only on UID of a card and nothing more. 

# Requirements
This is what you need to accomplish this project:
- 2004 I2C Display
- Encoder
- Arduino Nano
- MFRC522

# Wiring

- Display wiring:
  GND - GND, VCC - 5V, SDA - A4, SCL - A5
- Encoder wiring:
  GND - GND, VCC - 5V, S1 - D3, S2 - D2, KEY - D4
- RFID module wiring:
  GND - GND, 3.3V - 3.3V, RST - D9, MISO - D12, MOSI - D11, SCK - D13, SDA - D10

# Images

Main menu:

![Main menu](https://github.com/andyparkers/RFID-1356-MHz-UID-changer/blob/main/Images/IMG_20220729_220914.jpg)

Read UID:

![UID](https://github.com/andyparkers/RFID-1356-MHz-UID-changer/blob/main/Images/IMG_20220729_220856.jpg)

Write UID manually:

![Write UID](https://github.com/andyparkers/RFID-1356-MHz-UID-changer/blob/main/Images/IMG_20220729_221002.jpg)
