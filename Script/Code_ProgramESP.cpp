#include <WiFi.h>

WiFiClient client;

// Pin Ultrasonik + LED
#define TRIG_PIN 23
#define ECHO_PIN 22
#define LED_PIN 2   // LED internal ESP32 biasanya GPIO2

// Konstanta
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

// Variabel global
long duration;
float distanceCm;
float distanceInch;

// Konfigurasi WiFi & ThingSpeak
const char* ssid = "ITS-WIFI-TW2";
const char* password = "itssurabaya";
String thingSpeakAddress = "api.thingspeak.com";
String writeAPIKey = "I5IQULSC8CA6SDWU";

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  // Connect WiFi
  WiFi.disconnect();
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // --- Bagian Sensor ---
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(20);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(100);
  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH, 30000); // timeout 30ms (max ~5m)

  if (duration > 0) {
    distanceCm = duration * SOUND_SPEED / 2;
    distanceInch = distanceCm * CM_TO_INCH;
  } else {
    distanceCm = 0;
    distanceInch = 0;
  }

  Serial.print("Jarak: ");
  Serial.print(distanceCm);
  Serial.println(" cm");

  // --- Bagian LED ---
  if (distanceCm > 0 && distanceCm < 10) {
    digitalWrite(LED_PIN, HIGH);
    delay(150);
    digitalWrite(LED_PIN, LOW);
    delay(150);
  } else {
    digitalWrite(LED_PIN, LOW);
  }

  // --- Kirim ke ThingSpeak (1x tiap 15 detik agar sesuai limit) ---
  kirim_thingspeak(distanceCm, distanceInch);

  Serial.println("--------------------");
  delay(15000); // 15 detik (sesuai batas free ThingSpeak)
}

void kirim_thingspeak(float discm, float disinch) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi terputus, mencoba reconnect...");
    WiFi.begin(ssid, password);
    unsigned long t = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - t < 10000) {
      delay(200);
      Serial.print(".");
    }
    Serial.println();
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Reconnect gagal, data tidak terkirim");
      return;
    }
  }

  if (client.connect(thingSpeakAddress.c_str(), 80)) {
    String request = "/update?";
    request += "api_key=" + writeAPIKey;
    request += "&field1=" + String(discm);
    request += "&field2=" + String(disinch);

    String httpRequest = String("GET ") + request + " HTTP/1.1\r\n" +
                         "Host: " + thingSpeakAddress + "\r\n" +
                         "Connection: close\r\n\r\n";

    Serial.println("Mengirim ke ThingSpeak:");
    Serial.println(httpRequest);

    client.print(httpRequest);

    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println(">>> Client Timeout !");
        client.stop();
        return;
      }
    }

    while (client.available()) {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }

    Serial.println();
    Serial.println("Closing connection");
    client.stop();
  } else {
    Serial.println("Gagal connect ke api.thingspeak.com");
  }
}
