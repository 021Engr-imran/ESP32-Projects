/******************************************************|
 | * ESP32 Home Automation using Telegram              |
 | * Sensors: LM35, DHT22, Magnetic Switch             |
 | * install the libraries                             |
 |   UniversalTelegramBot.h                            |
 |     DHT.h                                           |
 |     ESP32Servo.h                                    |
 | *please add the wifi and telegram credentials       |
 ******************( Engr Imran )***********************/
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <DHT.h>
#include <ESP32Servo.h>

// ---------------- WiFi Credentials ----------------
const char* ssid = "YOUR_WIFI_NAME";                     // Enter your wifi name
const char* password = "YOUR_WIFI_PASSWORD";            // Eneter wifi Pasword

// ---------------- Telegram Bot -------------------
#define BOT_TOKEN "YOUR_TELEGRAM_BOT_TOKEN"             // Enter Telegram Bot Token
#define CHAT_ID   "YOUR_CHAT_ID"                        // Enter Telegram Chat ID

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

// ---------------- Pin Definitions ----------------
#define LM35_PIN     34
#define DHT_PIN      4
#define DHT_TYPE     DHT22
#define MAGNET_PIN   27
#define SERVO_PIN    26

// ---------------- Objects ----------------
DHT dht(DHT_PIN, DHT_TYPE);
Servo windowServo;

// ---------------- Thresholds ----------------
float TEMP_LIMIT = 35.0;     // Celsius
float HUM_LIMIT  = 70.0;     // Percent

// ---------------- Variables ----------------
unsigned long lastTimeBotRan = 0;
const long botDelay = 1000;

bool windowClosed = false;
bool alertTempSent = false;
bool alertHumSent = false;

// ---------------- Setup ----------------
void setup() {
  Serial.begin(115200);

  pinMode(MAGNET_PIN, INPUT_PULLUP);

  dht.begin();

  windowServo.attach(SERVO_PIN);
  windowServo.write(0); // Window open position

  // WiFi Connection
  WiFi.begin(ssid, password);
  secured_client.setInsecure();

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected");

  bot.sendMessage(CHAT_ID, "🏠 Home Automation System Started", "");
}

// ---------------- Read LM35 Temperature ----------------
float readLM35() {
  int adcValue = analogRead(LM35_PIN);
  float voltage = adcValue * (3.3 / 4095.0);
  float temperature = voltage * 100.0;
  return temperature;
}

// ---------------- Telegram Message Handler ----------------
void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {

    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;

    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }

    if (text == "/start") {
      bot.sendMessage(chat_id,
        "Welcome to ESP32 Home Automation\n"
        "/status\n"
        "/temperature\n"
        "/humidity\n"
        "/window\n"
        "/close_window", "");
    }

    if (text == "/temperature") {
      float t = readLM35();
      bot.sendMessage(chat_id, "🌡 Temperature: " + String(t) + " °C", "");
    }

    if (text == "/humidity") {
      float h = dht.readHumidity();
      bot.sendMessage(chat_id, "💧 Humidity: " + String(h) + " %", "");
    }

    if (text == "/window") {
      int state = digitalRead(MAGNET_PIN);
      if (state == LOW)
        bot.sendMessage(chat_id, "🪟 Window is CLOSED", "");
      else
        bot.sendMessage(chat_id, "🪟 Window is OPEN", "");
    }

    if (text == "/close_window") {
      windowServo.write(90); // Close window
      windowClosed = true;
      bot.sendMessage(chat_id, "🔒 Window closed remotely", "");
    }

    if (text == "/status") {
      float t = readLM35();
      float h = dht.readHumidity();
      int state = digitalRead(MAGNET_PIN);

      String status = "📊 System Status\n";
      status += "Temperature: " + String(t) + " °C\n";
      status += "Humidity: " + String(h) + " %\n";
      status += "Window: ";
      status += (state == LOW) ? "CLOSED" : "OPEN";

      bot.sendMessage(chat_id, status, "");
    }
  }
}

void loop() {

  // -------- Read Sensors --------
  float temperature = readLM35();
  float humidity = dht.readHumidity();
  int windowState = digitalRead(MAGNET_PIN);

  // -------- Temperature Alert --------
  if (temperature > TEMP_LIMIT && !alertTempSent) {
    bot.sendMessage(CHAT_ID, "⚠ High Temperature Alert: " + String(temperature) + " °C", "");
    alertTempSent = true;
  }
  if (temperature <= TEMP_LIMIT) alertTempSent = false;

  // -------- Humidity Alert --------
  if (humidity > HUM_LIMIT && !alertHumSent) {
    bot.sendMessage(CHAT_ID, "⚠ High Humidity Alert: " + String(humidity) + " %", "");
    alertHumSent = true;
  }
  if (humidity <= HUM_LIMIT) alertHumSent = false;

  // -------- Window Open Alert --------
  if (windowState == HIGH && !windowClosed) {
    bot.sendMessage(CHAT_ID, "🚨 Window is OPEN!", "");
    windowClosed = true;
  }
  if (windowState == LOW) windowClosed = false;

  // -------- Telegram Bot Check --------
  if (millis() - lastTimeBotRan > botDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}
