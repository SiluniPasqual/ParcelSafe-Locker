# ParcelSafe - Secure Delivery Locker System 
An automated, IoT-enabled secure delivery locker system designed to prevent parcel theft and missed deliveries by integrating real-time user verification through the Telegram app.

## Key Features
*   **Secure Code Authentication:** Delivery personnel enter a preset access code via a 4x3 matrix keypad.
*   **Telegram App Integration:** Sends instant notifications to the recipient's smartphone upon code entry, allowing them to grant (`/YES`) or deny (`/NO`) access remotely.
*   **Automated IR Detection:** Uses an Infrared (IR) sensor to detect the presence of a parcel and trigger alerts.
*   **Dual-Power Management:** Powered by two separate 3.7V Li-Ion rechargeable batteries isolated with boost converters (providing 5V for control logic and 12V for the lock).
*   **Wi-Fi Configuration Portal:** Utilizes WiFiManager on the ESP8266 to store network credentials securely in EEPROM.
*   **Master Password Protection:** Allows the owner to change the delivery access code manually using a master password sequence.

## Components & Hardware
*   **Microcontroller:** PIC16F877A (Core control processing)
*   **Wi-Fi Module:** NodeMCU ESP8266 (Wi-Fi connectivity & Telegram communication)
*   **Lock Mechanism:** 12V Solenoid Door Lock controlled via a 5V Relay module and transistor circuit
*   **Input/Output:** 4x3 Matrix Keypad, 16x2 LCD Display, Mini Buzzer, IR Sensor
*   **Power Supply:** 2x 3.7V 1800mA Li-Ion Batteries, MT3608 Boost Converters

## User Interface & System Workflow
Below is the system user interface and workflow displaying the Telegram notifications and LCD status prompts:

![User Interface](User%20Interface.png)

1. **Entry:** Delivery driver enters the code on the keypad.
2. **Notification:** The PIC microcontroller triggers the ESP8266 to send an interactive message via Telegram ("Welcome, grant access? /YES or /NO").
3. **Action:** If the user replies `/YES`, the relay triggers the solenoid lock to open. If `/NO`, access is denied.

## Group Members
*   Siluni Pasqual
*   Sasindu Basnayake
