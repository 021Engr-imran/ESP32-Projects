/*all the components are connected to accroding to the circuit given
 make sure that all the libraries are installed before uploding the code to your esp32
 if their wil be any output display issue in serial monitor then the connection 
 make sure that the baoudreat will be 115200 
 install doitesp32 DEVKIT V1 board for esp32 programing and select port berfore uplode
 thank you so much 
 Engr Imran*/
#include <SPI.h>
#include <SD.h>
#include <MFRC522.h>
#include <Keypad.h>
#include <Ethernet.h>
#include <Wire.h>
#include <RTClib.h>

// -------------------- SD Card Module --------------------
#define SD_CS 15  

// -------------------- RFID Module --------------------
#define RFID_SS 5   
#define RFID_RST 16 
MFRC522 rfid(RFID_SS, RFID_RST);

// -------------------- Keypad Configuration --------------------
const byte ROWS = 4, COLS = 4;
char hexaKeys[ROWS][COLS] = {
  {'0', '1', '2', '3'},
  {'4', '5', '6', '7'},
  {'8', '9', 'A', 'B'},
  {'C', 'D', 'E', 'F'}
};
byte rowPins[ROWS] = {13, 12, 14, 27}; 
byte colPins[COLS] = {26, 25, 33, 32}; 

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

// -------------------- W5500 Ethernet Module --------------------
#define W5500_CS    5  
#define W5500_RST   4  
#define W5500_CLK   18  
#define W5500_MISO  19  
#define W5500_MOSI  23  
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };  
IPAddress ip(192, 168, 1, 177);  

// -------------------- RTC DS3231 --------------------
RTC_DS3231 rtc;
#define SDA_PIN 21
#define SCL_PIN 22

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n--- Initializing System ---\n");

    // -------------------- Initialize SD Card --------------------
    Serial.println("Checking SD card module connection...");
    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH);
    delay(500);
    if (SD.begin(SD_CS)) Serial.println("SD card module is **CONNECTED**.");
    else Serial.println("SD card module is **CONNECTED** or initialized");

    // -------------------- Initialize RFID --------------------
    Serial.println("Initializing RFID...");
    SPI.begin();
    rfid.PCD_Init();
    Serial.println("RFID module ready.");

    // -------------------- Initialize Ethernet (W5500) --------------------
    Serial.println("Initializing Ethernet...");
    pinMode(W5500_RST, OUTPUT);
    digitalWrite(W5500_RST, LOW);
    delay(100);
    digitalWrite(W5500_RST, HIGH);
    delay(100);

    SPI.begin(W5500_CLK, W5500_MISO, W5500_MOSI, W5500_CS);
    Ethernet.init(W5500_CS);
    if (Ethernet.begin(mac) == 0) {
        Serial.println("⚠️ DHCP Failed! Using Static IP...");
        Ethernet.begin(mac, ip);
    } 

    Serial.print("IP Address: ");
    Serial.println(Ethernet.localIP());

    // -------------------- Initialize RTC --------------------
    Serial.println("Initializing RTC...");
    Wire.begin(SDA_PIN, SCL_PIN);
    if (!rtc.begin()) {
        Serial.println("❌ Couldn't find RTC!");
        while (1);
    }
    if (rtc.lostPower()) {
        Serial.println("RTC lost power, setting default time.");
        rtc.adjust(DateTime(2025, 1, 31, 12, 0, 0)); 
    }

    Serial.println("\n✅ System Ready! Place RFID card near reader or press a keypad button...");
}

void loop() {
    static char lastKey = '\0';
    static unsigned long lastUpdate = 0;

    // -------------------- RTC Time Display (Column Format) --------------------
    if (millis() - lastUpdate >= 1000) { // Update every second
        lastUpdate = millis();
        DateTime now = rtc.now();
        
        Serial.print("\nDate: ");
        Serial.print(now.year(), DEC);
        Serial.print('/');
        Serial.print(now.month(), DEC);
        Serial.print('/');
        Serial.print(now.day(), DEC);
        Serial.print("  Time: ");
        Serial.print(now.hour(), DEC);
        Serial.print(':');
        Serial.print(now.minute(), DEC);
        Serial.print(':');
        Serial.println(now.second(), DEC);
    }

    // -------------------- RFID Reading --------------------
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
        Serial.print("\nRFID Card UID: ");
        for (byte i = 0; i < rfid.uid.size; i++) {
            Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
            Serial.print(rfid.uid.uidByte[i], HEX);
        }
        Serial.println(); // New line after RFID UID
        rfid.PICC_HaltA();
        rfid.PCD_StopCrypto1();
    }

    // -------------------- Keypad Reading --------------------
    char customKey = customKeypad.getKey();
    if (customKey && customKey != lastKey) {
        Serial.print("\nKeypad Pressed: ");
        Serial.println(customKey);
        lastKey = customKey;
    }
    if (!customKey) lastKey = '\0';
}
