/* Project Skripsi 
 *  Sistem Deteksi Kebakaran Berbasis IoT
 *  Serial Communication between Arduino Uno and Wemos ESP32
*/


//Masukkan library yang dibutuhkan
#define BLYNK_PRINT Serial
#include <SoftwareSerial.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <FirebaseESP32.h>
BlynkTimer timer;

String sData; //menampung data dari arduino uno
String arrData[3]; //array digunakan untuk menjadi wadah setiap data yang sudah dipecah arduino
bool parsing = false; // parsing = memecah data

char auth[] = "KHeDst2U1qTlsxJh6EhDG5cAQhfjSIsP"; //Token BLYNK
char ssid[] = "Infinix HOT 11S NFC"; //SSID WiFi
char pass[] = "geoganteng1234"; //PASS WiFi

// Isikan sesuai pada Firebase
#define FIREBASE_HOST "https://skripsi-e4a2a-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "lPJQNlqVD3PN52en1oY7zIY3SfDP7BrTxag0fayD"

// mendeklarasikan objek data dari FirebaseESP8266
FirebaseData firebaseData;

//Connect to internet (port)
WiFiServer server(8080);
WiFiClient client;

//Function pengiriman data ke Blynk dan Firebase
void sendSensor() {
  while (Serial.available() > 0) { //fungsi untuk menampung data dari arduino
    char in = Serial.read(); //membaca setiap data (karakter per karakter)
    sData += in; //kemudian ditumpuk dalam stringData
  }

  sData.trim(); //ketika serial available sudah selesai maka data tsb kita anggap complete

  if (sData != "") {
    int index = 0;
    for (int i = 0; i <= sData.length(); i++) {
      char delimiter = '#';
      if (sData[i] != delimiter)
        arrData[index] += sData[i];
      else
        index++;
    }

    if (index == 2) {
      //Menampilkan pada widget di Blynk 
      Blynk.virtualWrite(V1, arrData[0]);
      Blynk.virtualWrite(V2, arrData[1]);
      Blynk.virtualWrite(V3, arrData[2]);
      delay(10);

      // Memberikan status suhu dan kelembaban kepada firebase
      if (Firebase.setString(firebaseData, "/Hasil_Pembacaan_1_Sensor/temperature", arrData[0])) {
        Serial.println("Temperature terkirim");
      } else {
        Serial.println("Temperature tidak terkirim");
        Serial.println("Karena: " + firebaseData.errorReason());
      }

      if (Firebase.setString(firebaseData, "/Hasil_Pembacaan_1_Sensor/gas", arrData[1])) {
        Serial.println("Gas terkirim");
      } else {
        Serial.println("Gas tidak terkirim");
        Serial.println("Karena: " + firebaseData.errorReason());
      }

      if (Firebase.setString(firebaseData, "/Hasil_Pembacaan_1_Sensor/fire", arrData[2])) {
        Serial.println("Fire terkirim");
      } else {
        Serial.println("Fire tidak terkirim");
        Serial.println("Karena: " + firebaseData.errorReason());
      }
    }
    arrData[0] = "";
    arrData[1] = "";
    arrData[2] = "";
  }
  sData = "";

  Serial.println("minta");
}

void setup() {
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass,  "iot.serangkota.go.id", 8080);
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
  timer.setInterval(3000L, sendSensor);
}

void loop() {

  Blynk.run();
  timer.run();
}
