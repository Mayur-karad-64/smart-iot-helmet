#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <DHT.h>

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// =====================================================
// WIFI CONFIG
// =====================================================
const char* ssid = "DashingMSK";
const char* password = "7777777777";

// =====================================================
// FLASK SERVER
// =====================================================
String serverName =
"http://SERVER_IP:5000/helmet";

// =====================================================
// OLED CONFIG
// =====================================================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  -1
);

// =====================================================
// DHT11
// =====================================================
#define DHTPIN D5
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// =====================================================
// MQ135
// =====================================================
#define MQ135_PIN A0

int gasLevel = 0;

// =====================================================
// THRESHOLDS
// =====================================================
int gasThreshold = 430;

float tempThreshold = 36.0;

// =====================================================
// BUZZER
// =====================================================
#define BUZZER_PIN D6

// =====================================================
// MPU6050
// =====================================================
Adafruit_MPU6050 mpu;

// =====================================================
// VARIABLES
// =====================================================
float temperature = 0;

float humidity = 0;

float angleX = 0;

float angleY = 0;

bool gasAlert = false;

bool tempAlert = false;

bool fallDetected = false;

// =====================================================
// GPS
// =====================================================
float latitude = 19.900562;

float longitude = 74.494852;

// =====================================================
// TIMER
// =====================================================
unsigned long previousMillis = 0;

const long interval = 2000;

// =====================================================
// SETUP
// =====================================================
void setup() {

  Serial.begin(115200);

  // =====================================================
  // BUZZER
  // =====================================================
  pinMode(BUZZER_PIN, OUTPUT);

  // =====================================================
  // WIFI
  // =====================================================
  WiFi.begin(ssid, password);

  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);

    Serial.print(".");
  }

  Serial.println("");

  Serial.println("WiFi Connected");

  Serial.print("ESP IP: ");

  Serial.println(WiFi.localIP());

  // =====================================================
  // I2C
  // =====================================================
  Wire.begin(D2, D1);

  Wire.setClock(50000);

  // =====================================================
  // OLED
  // =====================================================
  if (!display.begin(
        SSD1306_SWITCHCAPVCC,
        0x3C
      )) {

    Serial.println("OLED FAILED");

    while (1);
  }

  display.clearDisplay();

  display.setTextColor(WHITE);

  display.setTextSize(1);

  display.setCursor(10,20);

  display.println("SMART HELMET");

  display.setCursor(10,35);

  display.println("SYSTEM READY");

  display.display();

  delay(2000);

  // =====================================================
  // DHT11
  // =====================================================
  dht.begin();

  // =====================================================
  // MPU6050
  // =====================================================
  if (!mpu.begin()) {

    Serial.println("MPU6050 FAILED");

    while (1);
  }

  // =====================================================
  // STARTUP BEEP
  // =====================================================
  tone(BUZZER_PIN, 2000);

  delay(300);

  noTone(BUZZER_PIN);

  Serial.println("SYSTEM READY");
}

// =====================================================
// LOOP
// =====================================================
void loop() {

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {

    previousMillis = currentMillis;

    // =====================================================
    // DHT11
    // =====================================================
    temperature = dht.readTemperature();

    humidity = dht.readHumidity();

    // =====================================================
    // MQ135
    // =====================================================
    gasLevel = analogRead(MQ135_PIN);

    // =====================================================
    // MPU6050
    // =====================================================
    sensors_event_t a, g, temp;

    mpu.getEvent(&a, &g, &temp);

    angleX = atan2(
                 a.acceleration.y,
                 a.acceleration.z
             ) * 180 / PI;

    angleY = atan2(
                 -a.acceleration.x,
                 sqrt(
                   a.acceleration.y *
                   a.acceleration.y +

                   a.acceleration.z *
                   a.acceleration.z
                 )
             ) * 180 / PI;

    // =====================================================
    // ALERT CONDITIONS
    // =====================================================
    gasAlert = gasLevel > gasThreshold;

    tempAlert = temperature > tempThreshold;

    fallDetected =
      abs(angleX) > 45 ||
      abs(angleY) > 45;

    // =====================================================
    // ALERT MESSAGE
    // =====================================================
    String alertMessage = "NORMAL";

    if (gasAlert) {

      alertMessage = "GAS ALERT";
    }

    if (tempAlert) {

      alertMessage = "TEMP ALERT";
    }

    if (fallDetected) {

      alertMessage = "FALL ALERT";
    }

    // =====================================================
    // SERIAL MONITOR
    // =====================================================
    Serial.println("====================");

    Serial.print("Temperature: ");
    Serial.println(temperature);

    Serial.print("Humidity: ");
    Serial.println(humidity);

    Serial.print("Gas: ");
    Serial.println(gasLevel);

    Serial.print("Angle X: ");
    Serial.println(angleX);

    Serial.print("Angle Y: ");
    Serial.println(angleY);

    Serial.print("Alert: ");
    Serial.println(alertMessage);

    // =====================================================
    // JSON DATA
    // =====================================================
    String jsonData = "{";

    jsonData += "\"temperature\":";
    jsonData += String(temperature);
    jsonData += ",";

    jsonData += "\"humidity\":";
    jsonData += String(humidity);
    jsonData += ",";

    jsonData += "\"gas\":";
    jsonData += String(gasLevel);
    jsonData += ",";

    jsonData += "\"fall\":\"";
    jsonData += String(
                  fallDetected ?
                  "YES" : "NO"
                );
    jsonData += "\",";

    jsonData += "\"alert\":\"";
    jsonData += alertMessage;
    jsonData += "\",";

    jsonData += "\"latitude\":";
    jsonData += String(latitude, 6);
    jsonData += ",";

    jsonData += "\"longitude\":";
    jsonData += String(longitude, 6);

    jsonData += "}";

    // =====================================================
    // SEND DATA
    // =====================================================
    if (WiFi.status() == WL_CONNECTED) {

      WiFiClient client;

      HTTPClient http;

      http.begin(client, serverName);

      http.addHeader(
        "Content-Type",
        "application/json"
      );

      Serial.println("Sending JSON:");

      Serial.println(jsonData);

      int httpResponseCode =
          http.POST(jsonData);

      Serial.print("HTTP Response: ");

      Serial.println(httpResponseCode);

      http.end();
    }

    // =====================================================
    // OLED DISPLAY
    // =====================================================
    display.clearDisplay();

    display.setTextSize(1);

    display.setTextColor(WHITE);

    display.setCursor(0,0);

    display.print("Temp: ");
    display.print(temperature);
    display.println(" C");

    display.print("Hum : ");
    display.print(humidity);
    display.println(" %");

    display.print("Gas : ");
    display.println(gasLevel);

    display.print("Fall: ");
    display.println(
      fallDetected ? "YES" : "NO"
    );

    display.print("Alert:");
    display.println(alertMessage);

    display.display();

    // =====================================================
    // BUZZER ALERTS
    // =====================================================

    // GAS ALERT
    if (gasAlert) {

      tone(BUZZER_PIN, 2000);

      delay(200);

      noTone(BUZZER_PIN);
    }

    // TEMP ALERT
    if (tempAlert) {

      tone(BUZZER_PIN, 1500);

      delay(500);

      noTone(BUZZER_PIN);
    }

    // FALL ALERT
    if (fallDetected) {

      for (int i = 0; i < 3; i++) {

        tone(BUZZER_PIN, 2500);

        delay(120);

        noTone(BUZZER_PIN);

        delay(100);
      }
    }
  }
}