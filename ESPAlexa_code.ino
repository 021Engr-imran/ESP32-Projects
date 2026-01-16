/************************************************************
 * Alexa Bartender (ESP32 Compatible, Espalexa + PWM)        
 * Code is totally change according to Espalexa instead of Fauxmo  
 * Required libraries:                                       
 *   1) Espalexa                                             
 *   2) Adafruit_NeoPixel.h                                  
 *   3) WiFi.h                                               
 *                                                           
 * Change the WiFi credentials before uploading to ESP32     
 *                                                           
 *                      Engr. Imran                                       
 ********************************************************/   

#include <Arduino.h>
#include <WiFi.h>
#include <Espalexa.h>
#include <Adafruit_NeoPixel.h>

// ====== WiFi credentials ======
#define WIFI_SSID "YourWiFiName"        // Enter your Wi-Fi name
#define WIFI_PASS "YourWiFiPassword"    // Enter your Wi-Fi password

// ====== Espalexa setup ======
Espalexa espalexa;

// ====== NeoPixel ======
#define PIXEL_PIN 32
#define NUMPIXELS 150
Adafruit_NeoPixel pixels(NUMPIXELS, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

// ====== Motor and Relay Pins ======
#define ENA 26
#define ENB 25
#define IN1 13
#define IN2 12
#define IN3 14
#define IN4 27
#define M1_BOTTOM 15
#define M1_TOP 4
#define M2_TOP 5
#define M2_BOTTOM 18
#define RELAY1 19
#define RELAY2 21
#define RELAY3 22
#define RELAY4 23
#define RELAY5 33

// ====== Alexa Device Names ======
#define ID_DRINK1 "Drink 1"
#define ID_DRINK2 "Drink 2"
#define ID_DRINK3 "Drink 3"
#define ID_DRINK4 "Drink 4"
#define ID_DRINK5 "Drink 5"
#define ID_DRINK6 "Drink 6"
#define ID_DRINK7 "Drink 7"

// ====== Variables ======
int drink = 0;
bool drinkFlag[8] = { false };
int m1 = 0, m2 = 0;
int motorSpeed = 180;

// ====== Function Prototypes ======
void handleDrink(int device, uint8_t brightness);
void wifiSetup();

// =========================================================
//  WiFi setup
// =========================================================
void wifiSetup() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.printf("[WIFI] Connecting to %s ", WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println();
  Serial.printf("[WIFI] Connected! IP: %s\n", WiFi.localIP().toString().c_str());
  delay(1000);
}

// =========================================================
//  Alexa device handler
// =========================================================
void handleDrink(int device, uint8_t brightness) {
  motorSpeed = map(brightness, 0, 255, 0, 255);
  drink = 1;

  memset(drinkFlag, 0, sizeof(drinkFlag));
  drinkFlag[device] = true;

  Serial.printf("[ALEXA] %s activated, brightness %d\n", 
                device == 1 ? ID_DRINK1 :
                device == 2 ? ID_DRINK2 :
                device == 3 ? ID_DRINK3 :
                device == 4 ? ID_DRINK4 :
                device == 5 ? ID_DRINK5 :
                device == 6 ? ID_DRINK6 : ID_DRINK7,
                brightness);
}

// =========================================================
//  Setup
// =========================================================
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== Alexa Bartender (ESP32 - Espalexa) ===");

  // Pin modes
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(RELAY3, OUTPUT);
  pinMode(RELAY4, OUTPUT);
  pinMode(RELAY5, OUTPUT);

  // Initialize off state
  digitalWrite(RELAY1, LOW);
  digitalWrite(RELAY2, LOW);
  digitalWrite(RELAY3, LOW);
  digitalWrite(RELAY4, LOW);
  digitalWrite(RELAY5, LOW);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  pinMode(M1_BOTTOM, INPUT_PULLUP);
  pinMode(M1_TOP, INPUT_PULLUP);
  pinMode(M2_TOP, INPUT_PULLUP);
  pinMode(M2_BOTTOM, INPUT_PULLUP);

  // Initialize NeoPixel
  pixels.begin();
  pixels.setBrightness(50);
  pixels.clear();
  pixels.show();

  // Connect WiFi
  wifiSetup();

  // ====== Register Alexa Devices ======
  espalexa.addDevice(ID_DRINK1, [](uint8_t b){ handleDrink(1, b); });
  espalexa.addDevice(ID_DRINK2, [](uint8_t b){ handleDrink(2, b); });
  espalexa.addDevice(ID_DRINK3, [](uint8_t b){ handleDrink(3, b); });
  espalexa.addDevice(ID_DRINK4, [](uint8_t b){ handleDrink(4, b); });
  espalexa.addDevice(ID_DRINK5, [](uint8_t b){ handleDrink(5, b); });
  espalexa.addDevice(ID_DRINK6, [](uint8_t b){ handleDrink(6, b); });
  espalexa.addDevice(ID_DRINK7, [](uint8_t b){ handleDrink(7, b); });

  espalexa.begin();
  Serial.println("[ALEXA] Devices registered successfully");
}

// =========================================================
//  Loop
// =========================================================
void loop() {
  espalexa.loop();

  static unsigned long last = millis();
  if (millis() - last > 5000) {
    last = millis();
    Serial.printf("[RAM] Free heap: %d bytes\n", ESP.getFreeHeap());
  }

  if (drink == 1) {
    // ----- Motor M1 -----
    if (digitalRead(M1_BOTTOM) && !digitalRead(M1_TOP)) m1 = 1;
    if (!digitalRead(M1_BOTTOM) && digitalRead(M1_TOP) && m1 == 1) m1 = 2;

    if (m1 == 1) {
      analogWrite(ENB, motorSpeed);
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, LOW);
    } else if (m1 == 2) {
      analogWrite(ENB, 0);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, LOW);
    }

    // ----- Motor M2 -----
    if (!digitalRead(M2_TOP) && digitalRead(M2_BOTTOM) && m1 == 2) m2 = 1;
    if (digitalRead(M2_TOP) && !digitalRead(M2_BOTTOM) && m1 == 2 && m2 == 1) m2 = 2;

    if (m2 == 1) {
      analogWrite(ENA, motorSpeed);
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
    }

    if (m2 == 2) {
      analogWrite(ENA, 0);
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      delay(100);

      // ==== Relay Control ====
      if (drinkFlag[1]) { digitalWrite(RELAY1, HIGH); delay(2500); digitalWrite(RELAY1, LOW); }
      else if (drinkFlag[2]) { digitalWrite(RELAY2, HIGH); delay(6000); digitalWrite(RELAY2, LOW); }
      else if (drinkFlag[3]) { digitalWrite(RELAY3, HIGH); delay(3000); digitalWrite(RELAY3, LOW); }
      else if (drinkFlag[4]) { digitalWrite(RELAY4, HIGH); delay(3000); digitalWrite(RELAY4, LOW); }
      else if (drinkFlag[5]) { digitalWrite(RELAY5, HIGH); delay(3000); digitalWrite(RELAY5, LOW); }
      else if (drinkFlag[6]) { digitalWrite(RELAY1, HIGH); digitalWrite(RELAY2, HIGH); delay(3500); digitalWrite(RELAY1, LOW); delay(3000); digitalWrite(RELAY2, LOW); }
      else if (drinkFlag[7]) { digitalWrite(RELAY3, HIGH); digitalWrite(RELAY4, HIGH); delay(3000); digitalWrite(RELAY3, LOW); delay(3000); digitalWrite(RELAY4, LOW); }

      delay(2000);

      // Move back up
      while (digitalRead(M2_TOP)) {
        analogWrite(ENA, motorSpeed);
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, HIGH);
      }
      analogWrite(ENA, 0);
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);

      while (digitalRead(M1_TOP)) {
        analogWrite(ENB, motorSpeed);
        digitalWrite(IN3, LOW);
        digitalWrite(IN4, HIGH);
      }
      analogWrite(ENB, 0);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, LOW);

      // Reset
      drink = 0; m1 = 0; m2 = 0;
      memset(drinkFlag, 0, sizeof(drinkFlag));
      pixels.clear(); pixels.show();
      Serial.println("[DONE] Cycle complete");
    }
  }
}
