/* Project Skripsi
    Sistem Deteksi Kebakaran Berbasis IoT
    Serial Communication between Arduino Uno and Wemos ESP32
*/

//Masukkan library yang dibutuhkan
#include "DHT.h"
#include <SoftwareSerial.h>
SoftwareSerial mySerial (3, 4); //RX, TX
#include <MQUnifiedsensor.h>

//Atur pake DHT tipe apa dan colok di pin brp
#define DHTPIN 6
#define DHTTYPE DHT21
DHT dht (DHTPIN, DHTTYPE);

//Set up untuk MQ2 (Base on example code for MQ2)
#define placa "Arduino UNO"
#define Voltage_Resolution 5
#define pin A0 //Analog input 0 of your arduino
#define type "MQ-2" //MQ2
#define ADC_Bit_Resolution 10 // For arduino UNO/MEGA/NANO
#define RatioMQ2CleanAir 9.83//RS / R0 = 9.83 ppm 

MQUnifiedsensor MQ2(placa, Voltage_Resolution, ADC_Bit_Resolution, pin, type);

//Pilihan untuk jenis gas apa yang ingin digunakan
// 6 jumlah elemen dalam setiap array dan 10 panjang maks karakter dalam 1 elemen
char jenisgas[6][10] = {"H2", "LPG", "CO", "Alcohol", "Propane"};
float gasA[6] = {987.99, 574.25, 36974, 3616.1, 658.71};
float gasB[6] = { -2.162, -2.222, -3.109, -2.675, -2.168};
int itemcheck = 1;

String data;
char c;

//Deklarasi untuk flame sensor
int fsensor = 7;
String firestatus = "";

int led = 9;
int buzzer = 8;
int valve = 10;

void setup() {
  Serial.begin(115200); //Baudrate harus sama dengan WeMos
  mySerial.begin(115200); //Baudrate untuk berkomunikasi dengan WeMos
  pinMode(fsensor, INPUT);//define F_Sensor input pin
  MQ2.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ2.setA(gasA[itemcheck]); MQ2.setB(gasB[itemcheck]); // Configurate the ecuation values to get CO2 concentration
  MQ2.init();
  Serial.print("Calibrating please wait.");
  float calcR0 = 0;
  for (int i = 1; i <= 10; i ++)
  {
    MQ2.update(); // Update data, the arduino will be read the voltage on the analog pin
    calcR0 += MQ2.calibrate(RatioMQ2CleanAir);
    Serial.print(".");
  }
  MQ2.setR0(calcR0 / 10);
  Serial.println("  done!.");
  if (isinf(calcR0)) {
    Serial.println("Warning: Conection issue founded, R0 is infite (Open circuit detected) please check your wiring and supply");
    while (1);
  }
  if (calcR0 == 0) {
    Serial.println("Warning: Conection issue founded, R0 is zero (Analog pin with short circuit to ground) please check your wiring and supply");
    while (1);
  }
  /*****************************  MQ CAlibration ********************************************/
  dht.begin();
  delay(10);
}

void loop() {
  pinMode(led, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(valve, OUTPUT);

  float t = dht.readTemperature();

  MQ2.update(); // Update data, the arduino will be read the voltage on the analog pin
  float hasil = MQ2.readSensor(); // Sensor will read PPM concentration using the model and a and b values setted before or in the setup
  //      Serial.print(jenisgas[itemcheck]);Serial.print(" : ");Serial.print(hasil);Serial.println(" PPM");

  int fire = digitalRead(fsensor);

  if (isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  //  if (t > 29)
  //  {
  //    digitalWrite(led, HIGH);
  //  }
  //  else {
  //    digitalWrite(led, LOW);
  //  }
  //
  //  if (hasil > 10)
  //  {
  //    digitalWrite(buzzer, HIGH);
  //  }
  //
  //  else {
  //    digitalWrite(buzzer, LOW);
  //  }
  //
  //  if(fire == LOW){
  //    digitalWrite(led, HIGH);
  //    digitalWrite(buzzer, HIGH);
  //    digitalWrite(valve, HIGH);
  //  }
  //
  //  else{
  //    digitalWrite(led, LOW);
  //    digitalWrite(buzzer, LOW);
  //    digitalWrite(valve, LOW);
  //  }

  if (t > 30 && hasil > 20) {
    digitalWrite(led, HIGH);
    digitalWrite(buzzer, HIGH);
    digitalWrite(valve, HIGH);
  } else if (t > 30) {
    digitalWrite(led, HIGH);
  } else {
    digitalWrite(led, LOW);
  }

  if (hasil > 20) {
    digitalWrite(buzzer, HIGH);
  } else {
    digitalWrite(buzzer, LOW);
  }

  if (t > 30 && hasil > 20 && fire == LOW) {
    digitalWrite(led, HIGH);
    digitalWrite(buzzer, HIGH);
    digitalWrite(valve, HIGH);
  } else {
    digitalWrite(valve, LOW);
  }

  while (mySerial.available() > 0) {
    c = mySerial.read();
    data += c;
  }

  data.trim();

  if (data.length() > 0) {
    Serial.println(data);
    if (data == "minta") { //Kata minta dari WeMos

      Serial.print(F("Temperature 1: "));
      Serial.print(t);
      Serial.print(F("°C Propane: "));
      Serial.print(hasil);

      mySerial.print(t);
      mySerial.print('#');
      mySerial.print(hasil);
      mySerial.print('#');
      mySerial.print(firestatus);

      if (fire == LOW) {
        firestatus = " Fire Detected";
        Serial.print(firestatus);
        //        delay(3000);
      }
      else
      {
        firestatus = " No Fire";
        Serial.print(firestatus);
      }
    } else {
      Serial.println("waiting for request from ESP32"); //Menunggu request dari WeMos
    }

    data = "";
  }
}
