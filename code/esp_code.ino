#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <WiFiManager.h>

#define SWITCH_PIN D5
#define LED_PIN 2
#define EEPROM_SIZE 64

#define picToEsp D1   // This goes to PORTC.F1 which is RC1
#define espTopic D6   // This goes to PORTC.F2 which is RC2
#define espToPic2 D7  // This goes to PORTC.F3 which is RC3

#define BOT_TOKEN "7494735194:AAHcK4J6GphTRzZKF7Y0chH3h7qucQVlCQI"

const unsigned long BOT_MTBS = 200; // Mean time between scan messages

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long bot_lasttime;
int ledStatus = 0;

WiFiManager wifiManager;

void setup() {
    Serial.begin(9600);
    pinMode(SWITCH_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
    pinMode(picToEsp, INPUT);
    pinMode(espTopic, OUTPUT);
    pinMode(espToPic2, OUTPUT);

    digitalWrite(LED_PIN, LOW); // Turn off LED initially

    // Initialize EEPROM
    EEPROM.begin(EEPROM_SIZE);

    // Check if we need to configure WiFi
    bool configWiFi = digitalRead(SWITCH_PIN) == LOW || !loadWiFiCredentials();
    if (configWiFi) {
        Serial.println("Starting WiFi configuration...");
        startConfigPortal();
    }

    Serial.print("Connected to WiFi SSID: ");
    secured_client.setInsecure(); // Disable certificate validation
    Serial.println(WiFi.SSID());
    blinkLED(2); // Blink LED twice to indicate successful connection
}

void loop() {
    digitalWrite(espTopic, 0);
    digitalWrite(espToPic2, 0);

    // Check if the switch is pressed
    if (digitalRead(SWITCH_PIN) == LOW) {
        Serial.println("Switch pressed. Starting WiFi configuration...");
        startConfigPortal();
        while (digitalRead(SWITCH_PIN) == LOW) {
            blinkLED(2);
        }
    }

    if (digitalRead(picToEsp) == HIGH) {
        Serial.println("Detected high");
        String chat_id = "848554756";
        String welcome = "Welcome Ms. Pasqual, Grant access to the box?.\n\n";
        welcome += "/YES: to UNLOCK the box\n\n";
        welcome += "/NO: to LOCK the box\n";

        bot.sendMessage(chat_id, welcome, "Markdown");

        // Wait until picToEsp pin goes low
        while (digitalRead(picToEsp) == HIGH) {
            if (millis() - bot_lasttime > BOT_MTBS) {
                int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
                while (numNewMessages) {
                    Serial.println("got response");
                    for (int i = 0; i < numNewMessages; i++) {
                        String text = bot.messages[i].text;
                        if (text == "/YES") {
                            bot.sendMessage(chat_id, "Box Unlocked", "");
                            digitalWrite(espTopic, HIGH);
                            digitalWrite(espToPic2, HIGH);
                            delay(20);
                            digitalWrite(LED_PIN, LOW);
                        } else if (text == "/NO") {
                            bot.sendMessage(chat_id, "Box Locked", "");
                            digitalWrite(espTopic, HIGH);
                            digitalWrite(espToPic2, LOW);
                            digitalWrite(LED_PIN, HIGH);
                        }
                    }
                    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
                }
                bot_lasttime = millis();
            }
        }
    }

    // Main message processing
    if (millis() - bot_lasttime > BOT_MTBS) {
        int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        while (numNewMessages) {
            Serial.println("got response");
            handleNewMessages(numNewMessages);
            numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        }
        bot_lasttime = millis();
    }
}

bool loadWiFiCredentials() {
    char ssid[32];
    char password[32];

    for (int i = 0; i < 32; ++i) {
        ssid[i] = EEPROM.read(i);
        password[i] = EEPROM.read(32 + i);
    }

    if (ssid[0] == '\0' || password[0] == '\0') {
        Serial.println("WiFi credentials not found in EEPROM");
        return false;
    }

    WiFi.begin(ssid, password);

    int attempts = 10;
    while (WiFi.status() != WL_CONNECTED && attempts > 0) {
        delay(1000);
        Serial.print(".");
        attempts--;

        if (digitalRead(SWITCH_PIN) == LOW) {
            Serial.println("\nSwitch pressed during connection attempt. Clearing EEPROM and resetting.");
            clearEEPROM();
            ESP.restart();
        }
    }

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Failed to connect to WiFi");
        return false;
    }

    Serial.print("Connected to WiFi SSID: ");
    Serial.println(WiFi.SSID());
    return true;
}

void saveWiFiCredentials() {
    String ssid = WiFi.SSID();
    String password = WiFi.psk();

    for (int i = 0; i < 32; ++i) {
        EEPROM.write(i, i < ssid.length() ? ssid[i] : '\0');
        EEPROM.write(32 + i, i < password.length() ? password[i] : '\0');
    }

    EEPROM.commit();
    Serial.println("WiFi credentials saved to EEPROM");
}

void startConfigPortal() {
    wifiManager.resetSettings();
    wifiManager.setConfigPortalTimeout(180);

    bool portalRunning = true;
    while (portalRunning) {
        if (digitalRead(SWITCH_PIN) == LOW) {
            Serial.println("\nSwitch pressed during configuration portal. Clearing EEPROM and resetting.");
            clearEEPROM();
            ESP.restart();
        }

        if (wifiManager.autoConnect("Ms Pasqual's")) {
            Serial.println("Connected to WiFi");
            saveWiFiCredentials();
            portalRunning = false;
        } else {
            Serial.println("Failed to connect and hit timeout");
            delay(3000);
            ESP.restart();
        }
    }
}

void clearEEPROM() {
    for (int i = 0; i < EEPROM_SIZE; ++i) {
        EEPROM.write(i, 0);
    }
    EEPROM.commit();
    Serial.println("EEPROM cleared");
}

void blinkLED(int times) {
    for (int i = 0; i < times; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(200);
        digitalValue: digitalWrite(LED_PIN, LOW); // safe cleanup
        delay(200);
    }
}

void handleNewMessages(int numNewMessages) {
    Serial.print("handleNewMessages ");
    Serial.println(numNewMessages);
    for (int i = 0; i < numNewMessages; i++) {
        String chat_id = bot.messages[i].chat_id;
        String text = bot.messages[i].text;
        String from_name = bot.messages[i].from_name;
        if (from_name == "") from_name = "Guest";

        if (text == "/ledon") {
            digitalWrite(LED_PIN, 0);
            ledStatus = 1;
            bot.sendMessage(chat_id, "Led is ON", "");
        }
        if (text == "/ledoff") {
            ledStatus = 0;
            digitalWrite(LED_PIN, 1);
            bot.sendMessage(chat_id, "Led is OFF", "");
        }
        if (text == "/status") {
            if (ledStatus) {
                bot.sendMessage(chat_id, "Led is ON", "");
            } else {
                bot.sendMessage(chat_id, "Led is OFF", "");
            }
        }
        if (text == "/start") {
            String welcome = "Welcome Ms. Pasqual to Universal Arduino Telegram Bot library.\n";
            welcome += "This is Flash Led Bot example.\n\n";
            welcome += "/ledon : to switch the Led ON\n";
            welcome += "/ledoff : to switch the Led OFF\n";
            welcome += "/status : Returns current status of LED\n";
            bot.sendMessage(chat_id, welcome, "Markdown");
        }
    }
}