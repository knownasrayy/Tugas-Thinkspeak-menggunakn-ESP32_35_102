# Proyek IoT: ESP32 + Sensor Ultrasonik + ThingSpeak

Proyek ini mendemonstrasikan cara membuat sistem pengukuran jarak berbasis **ESP32** dan **sensor ultrasonik HC-SR04**, lalu mengirimkan datanya ke platform cloud **ThingSpeak** untuk monitoring secara *real-time*.

## 👨‍💻 Kelompok
- **Zein Muhammad Hasan** - 5027241035
- **Rayhan Agnan Kusuma** - 5027241102

---

## 🎯 Tujuan Proyek
1.  Membuat sistem pengukuran jarak menggunakan sensor ultrasonik berbasis ESP32.
2.  Mengirimkan data jarak ke platform ThingSpeak untuk pemantauan jarak jauh.
3.  Memberikan indikator LED sebagai penanda ketika jarak objek terlalu dekat.

---

## 🛠️ Alat dan Bahan
* ESP32 Dev Module
* Sensor Ultrasonik HC-SR04
* LED (menggunakan LED internal GPIO2 pada ESP32)
* Arduino IDE (untuk pemrograman)
* Akun ThingSpeak

---

## ⚙️ Implementasi

### 1. Konfigurasi ThingSpeak
- **Buat Channel Baru**: Login ke ThingSpeak, buat *Channel* baru.
- **Konfigurasi Fields**: Tambahkan `Field1` untuk menyimpan data jarak dalam `cm` dan `Field2` untuk jarak dalam `inch`.
- **Salin API Key**: Buka tab **API Keys** dan salin **Write API Key** untuk digunakan dalam kode.

### 2. Kode Program
Kode berikut di-upload ke ESP32 menggunakan Arduino IDE. Pastikan untuk mengganti `ssid`, `password`, dan `writeAPIKey` sesuai dengan konfigurasi jaringan dan akun ThingSpeak Anda.

```cpp
#include <WiFi.h>

WiFiClient client;

// Pinout Konfigurasi
#define TRIG_PIN 23
#define ECHO_PIN 22
#define LED_PIN  2   // LED internal ESP32

// Konstanta Fisika
#define SOUND_SPEED 0.034
#define CM_TO_INCH  0.393701

// Variabel Global
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

  // Inisialisasi koneksi WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // 1. Membaca Sensor Ultrasonik
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH, 30000); // Timeout 30ms

  // Konversi durasi ke jarak
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

  // 2. Kontrol LED Indikator
  if (distanceCm > 0 && distanceCm < 10) {
    digitalWrite(LED_PIN, HIGH); // LED berkedip jika objek terlalu dekat
    delay(150);
    digitalWrite(LED_PIN, LOW);
    delay(150);
  } else {
    digitalWrite(LED_PIN, LOW);
  }

  // 3. Mengirim Data ke ThingSpeak
  kirimKeThingSpeak(distanceCm, distanceInch);
  
  delay(15000); // Jeda 15 detik sesuai limit gratis ThingSpeak
}

void kirimKeThingSpeak(float distCm, float distInch) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Koneksi WiFi terputus. Data tidak terkirim.");
    return;
  }

  if (client.connect(thingSpeakAddress.c_str(), 80)) {
    String request = "/update?api_key=" + writeAPIKey +
                     "&field1=" + String(distCm) +
                     "&field2=" + String(distInch);

    String httpRequest = "GET " + request + " HTTP/1.1\r\n" +
                         "Host: " + thingSpeakAddress + "\r\n" +
                         "Connection: close\r\n\r\n";

    client.print(httpRequest);
    Serial.println("Data terkirim ke ThingSpeak.");
    client.stop();
  } else {
    Serial.println("Gagal terhubung ke ThingSpeak API.");
  }
}
```

---

## 📈 Hasil Proyek

### Tampilan Dashboard ThingSpeak
Data jarak berhasil dikirim dan divisualisasikan dalam bentuk grafik secara *real-time*. Interval pengiriman data adalah setiap 15 detik.
*(Letakkan screenshot dashboard ThingSpeak Anda di sini)*

``

### Tampilan Serial Monitor
Serial monitor pada Arduino IDE menunjukkan proses koneksi WiFi, pengukuran jarak, dan status pengiriman data ke ThingSpeak.
*(Letakkan screenshot serial monitor Anda di sini)*

``

---

## ✅ Kesimpulan
Proyek ini berhasil menunjukkan bahwa ESP32 dapat diintegrasikan dengan sensor ultrasonik untuk mengukur jarak dan mengirimkan data secara nirkabel ke platform cloud seperti ThingSpeak. Sistem ini efektif untuk pemantauan jarak jauh dan dilengkapi dengan indikator LED lokal untuk peringatan langsung.
