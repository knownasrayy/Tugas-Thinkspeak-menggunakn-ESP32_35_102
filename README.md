Laporan Proyek IoT: ESP32 + Sensor Ultrasonik + ThingSpeakKelompokZein Muhammad Hasan - 5027241035Rayhan Agnan Kusuma - 50272411021. PendahuluanInternet of Things (IoT) merupakan teknologi yang memungkinkan perangkat elektronik terhubung ke internet untuk mengirimkan dan menerima data. Pada proyek ini, digunakan ESP32 yang dilengkapi dengan sensor ultrasonik untuk mengukur jarak. Data hasil pengukuran dikirimkan ke ThingSpeak, sebuah platform cloud IoT untuk monitoring data secara real-time.Tujuan Proyek:Membuat sistem pengukuran jarak menggunakan sensor ultrasonik berbasis ESP32.Mengirimkan data jarak ke platform ThingSpeak untuk pemantauan jarak jauh.Memberikan indikator LED sebagai penanda ketika jarak objek terlalu dekat.2. Alat dan BahanESP32 Dev ModuleSensor Ultrasonik HC-SR04LED (menggunakan LED internal GPIO2 pada ESP32)Arduino IDE (untuk pemrograman)Akun ThingSpeak3. Implementasi3.1 Konfigurasi ThingSpeakLogin ke ThingSpeak.Buat Channel baru dengan menambahkan Field1 (Distance dalam cm) dan Field2 (Distance dalam inch).Buka tab API Keys → salin Write API Key.3.2 Kode ProgramBerikut adalah kode lengkap untuk proyek ini.#include <WiFi.h>

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
3.3 Penjelasan KodeBagian WiFiWiFi.begin(ssid, password);
while (WiFi.status() != WL_CONNECTED) { ... }
→ ESP32 akan terhubung ke jaringan WiFi. Jika sukses, IP Address ditampilkan.Bagian Sensor UltrasonikdigitalWrite(TRIG_PIN, HIGH);
delayMicroseconds(100);
duration = pulseIn(ECHO_PIN, HIGH, 30000);
distanceCm = duration * SOUND_SPEED / 2;
→ Mengirim pulsa dari pin TRIG, menerima pantulan di pin ECHO, lalu dihitung jaraknya dengan kecepatan suara (0.034 cm/µs).Bagian LEDif (distanceCm > 0 && distanceCm < 10) {
    digitalWrite(LED_PIN, HIGH);
    ...
}
→ Jika objek terlalu dekat (< 10 cm), LED akan berkedip.Bagian Kirim Data ke ThingSpeakString request = "/update?api_key=...&field1=...&field2=...";
client.print(httpRequest);
→ Mengirim data ke channel ThingSpeak via protokol HTTP GET. Field1 menyimpan jarak dalam cm, Field2 dalam inch.Interval Waktudelay(15000);
→ Data dikirim tiap 15 detik, sesuai dengan limit akun gratis ThingSpeak.4. HasilESP32 berhasil membaca jarak dari sensor ultrasonik.Jika jarak < 10 cm, LED berkedip sebagai indikator.Data jarak terkirim ke ThingSpeak dan dapat dilihat dalam bentuk grafik (field1 = cm, field2 = inch).5. KesimpulanProyek ini membuktikan bahwa:ESP32 dapat digunakan untuk menghubungkan sensor ultrasonik dengan layanan IoT cloud.ThingSpeak efektif untuk memantau data secara real-time dengan visualisasi grafik.Sistem indikator LED memberikan tambahan informasi lokal tanpa harus melihat dashboard ThingSpeak.6. Dokumentasi Hasil PekerjaanTampilan di ThingSpeakDari sini terlihat bahwa data jarak sudah berhasil terkirim dengan rentang waktu setiap 15 detik.[Gambar dasbor ThingSpeak menunjukkan grafik data jarak]Tampilan di Serial Monitor[Gambar monitor serial Arduino IDE menunjukkan output jarak]
