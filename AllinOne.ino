#define BLYNK_PRINT Serial

#define BLYNK_TEMPLATE_ID "TMPL6MrVrGpr4"
#define BLYNK_TEMPLATE_NAME "Medical Check Up Detak Jantung dan Tekanan Darah"
#define BLYNK_AUTH_TOKEN "M_PFcwIgjlBTG2LWbO8mdojUuH-tkoAa"

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DFRobot_MAX30102.h>
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

char ssid[] = "LAN-ang";
char pass[] = "anaklanang";

LiquidCrystal_I2C lcd(0x27, 16, 2); // Address LCD I2C dan ukuran (16x2)
DFRobot_MAX30102 particleSensor;

// const int buttonPin0 = D5; // Pin trigger Tensi
const int buttonPin1 = D6;  // Pin push button Max
const int buttonPin2 = D7;  // Pin push button Tensi
const int triggerPin = D8; // Pin trigger Tensi
const int resetPin = D5;
bool buttonState = false; // Menyimpan status push button
bool systemActive = false; // Menyimpan status sistem aktif/non-aktif
bool buttonState1 = false; // Menyimpan status push button
bool systemActive1 = false; // Menyimpan status sistem aktif/non-aktif
bool fingerDetected = false;
bool systemMessageDisplayed = false;
bool readingInProgress = false; // Menyimpan status pembacaan tensi
bool initialDisplay = true; // Menyimpan status tampilan awal

unsigned long lastDebounceTime = 0; // Waktu terakhir tombol ditekan
unsigned long debounceDelay = 50; // Debounce time (ms)

char buff[64];
bool b_read = false, b_discard = false;
char discard;
int i, j = 0;
char final_buff[64];
int hexSys, hexDias;

int32_t SPO2;
int8_t SPO2Valid;
int32_t heartRate;
int8_t heartRateValid;

int Cons = 0;
int reset = 1;

void setup()
{

  Serial.begin(115200);
  pinMode(buttonPin1, INPUT_PULLUP); // Konfigurasi pin button dengan pull-up
  pinMode(buttonPin2, INPUT_PULLUP); // Konfigurasi pin button dengan pull-up
  pinMode(resetPin, INPUT_PULLUP); // Konfigurasi pin button dengan pull-up
  pinMode(triggerPin, OUTPUT); // Konfigurasi pin button dengan pull-up

  if (!particleSensor.begin())
  {
    Serial.println("MAX30102 was not found");
    while (1)
    {
      delay(1000);
    }
  }

  particleSensor.sensorConfiguration(50, SAMPLEAVG_4, MODE_MULTILED, SAMPLERATE_100, PULSEWIDTH_411, ADCRANGE_16384);

  lcd.init();
  lcd.backlight();
  lcd.clear();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
}


void loop(){
  int reset = digitalRead(resetPin);
  
  if (reset==LOW){
    initialDisplay = true;
    Cons = 0;
  }

  if (initialDisplay) {
    lcd.setCursor(0, 0);
    lcd.print("1: Max          ");
    lcd.setCursor(0, 1);
    lcd.print("2: Tensi        ");
    initialDisplay = false;
  }

  
  int reading = digitalRead(buttonPin1);
  Serial.println(reading);
  if (reading == LOW && (millis() - lastDebounceTime) > debounceDelay) {
    lastDebounceTime = millis();
    buttonState = !buttonState;
    systemActive = !systemActive;
    systemMessageDisplayed = false; // Reset flag

    clearLine(0); // Clear first line
    clearLine(1); // Clear second line
    if (systemActive) {
      lcd.setCursor(0, 0);
      lcd.print("Sistem Aktif    ");
    } else {
      lcd.setCursor(0, 0);
      lcd.print("Sistem Mati     ");
    }
    delay(1000); // Delay untuk menampilkan status sistem
    Cons = 1 ;

  }
  
  int reading1 = digitalRead(buttonPin2);
  if (reading1 == LOW && (millis() - lastDebounceTime) > debounceDelay) {
    lastDebounceTime = millis();
    buttonState1 = !buttonState1;
    systemActive1 = !systemActive1;
    systemMessageDisplayed = false; // Reset flag
    readingInProgress = false; // Reset flag

    clearLine(0); // Clear first line
    clearLine(1); // Clear second line
    if (systemActive1) {
      digitalWrite(triggerPin, HIGH);
      delay(200);
      digitalWrite(triggerPin, LOW);
      lcd.setCursor(0, 0);
      lcd.print("Sistem Aktif    ");
      Serial.println("Sistem Aktif");
      Serial.print("Memproses....  ");
      Serial.println("       ");
    } else {
      digitalWrite(triggerPin, HIGH);
      delay(200);
      digitalWrite(triggerPin, LOW);
      lcd.setCursor(0, 0);
      lcd.print("Sistem Mati     ");
      Serial.println("Sistem Mati");
      Serial.print("Tekan tombol untuk memulai.. ");
      Serial.println("       ");
      resetRXBuffer(); // Reset RX buffer and related variables
    }
    delay(1000); // Delay untuk menampilkan status sistem
    Cons = 2;
  }

  if (Cons==1){
    Max();
  }
  else if (Cons==2){
    Tensi();
  }
  else {
    Cons=0;
  }

  Blynk.run();
  return;
}

void Max()
{
  if (systemActive) {
    long irValue = particleSensor.getIR();
    Serial.print("IR=");
    Serial.print(irValue);

    if (irValue < 45000) {
      Serial.println("  No finger?");
      fingerDetected = false;

      lcd.setCursor(0, 0);
      lcd.print("No Finger       ");

    } else {
      if (!fingerDetected) {
        fingerDetected = true;
        clearLine(0);
        clearLine(1);
        lcd.print("                "); // Menghapus pesan "No Finger"
      }
      particleSensor.heartrateAndOxygenSaturation(&SPO2, &SPO2Valid, &heartRate, &heartRateValid);

      if (heartRate == -999) {
        Serial.println("letakkan jari");
        lcd.setCursor(0, 0);
        lcd.print("letakkan jari   ");
  
      
      } else {
        Serial.print(F("heartRate="));
        Serial.print(heartRate, DEC);
        Serial.print(F(", heartRateValid="));
        Serial.println(heartRateValid, DEC);

  
        lcd.setCursor(0, 0);
        lcd.print("HR:");
        lcd.print(heartRate);
        lcd.print(" bpm        ");

        Blynk.virtualWrite(V7, heartRate);

      }
    }

    Serial.println();
  } 
  
  else {
    if (!systemMessageDisplayed) {
   
      lcd.setCursor(0, 0);
      lcd.print("Tekan Tombol    ");
      lcd.setCursor(0, 1);
      lcd.print("untuk Aktif     ");
      systemMessageDisplayed = true; // Set flag to true after displaying the message
    }
  }
  Blynk.run();
}

void Tensi() {
  // Baca status push button dengan debouncing
    if (systemActive1) {
    if (!readingInProgress) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Memproses....   ");
      readingInProgress = true;
    }

    // Baca input serial
    while (Serial.available()) {
      if (!b_read) {
        buff[0] = Serial.read();
        if (buff[0] == 'e') {
          buff[1] = Serial.read();
          if (buff[1] == 'r') {
            buff[2] = Serial.read();
            if (buff[2] == 'r') {
              buff[3] = Serial.read();
              if (buff[3] == ':') {
                buff[4] = Serial.read();
                if (buff[4] == '0') {
                  b_read = true;
                  j = 0;
                  b_discard = false;
                  i = 0;
                }
              }
            }
          }
        }
      }
      if (b_read) {
        if (!b_discard) {
          discard = Serial.read();
          i++;
        } else if (j < 11) {
          final_buff[j] = Serial.read();
          j++;
        } else {
          b_read = false;
        }
        if (i == 30) {
          b_discard = true;
        }
      }
      delay(2);
    }

    if (j >= 11) {
      // Konversi karakter heksadesimal ke desimal
      hexSys = (final_buff[0] > '9' ? (final_buff[0] - '7') * 16 : (final_buff[0] - '0') * 16) +
               (final_buff[1] > '9' ? (final_buff[1] - '7') : (final_buff[1] - '0'));

      hexDias = (final_buff[3] > '9' ? (final_buff[3] - '7') * 16 : (final_buff[3] - '0') * 16) +
                (final_buff[4] > '9' ? (final_buff[4] - '7') : (final_buff[4] - '0'));

      // Tentukan kategori tekanan darah
      String kategori;
      if (hexSys < 0 || hexDias < 0) {
        kategori = "Tidak Terbaca";
      } else if (hexSys < 120 && hexDias < 80) {
        kategori = "Normal";
      } else if ((hexSys >= 120 && hexSys < 140) || (hexDias >= 80 && hexDias < 90)) {
        kategori = "Pra-HTN";
      } else if ((hexSys >= 140 && hexSys < 160) || (hexDias >= 90 && hexDias < 100)) {
        kategori = "HTN T1";
      } else if (hexSys >= 160 || hexDias >= 100) {
        kategori = "HTN T2";
      } else {
        kategori = "Unknown";
      }

      // Cetak ke Serial Monitor
      Serial.print("Sistolik: ");
      Serial.print(hexSys);
      Serial.print(" Diastolik: ");
      Serial.print(hexDias);
      Serial.print(" Kategori: ");
      Serial.println(kategori);

      // Cetak ke LCD
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("MmHg :");
      lcd.print(hexSys);
      lcd.setCursor(9, 0);
      lcd.print("/");
      lcd.print(hexDias);
      lcd.setCursor(0, 1);
      lcd.print(kategori);

      delay(500);
      readingInProgress = false; // Reset flag
    }
  } else {
    if (!systemMessageDisplayed) {
      lcd.setCursor(0, 0);
      lcd.print("Tekan Tombol    ");
      lcd.setCursor(0, 1);
      lcd.print("Untuk Memulai     ");
      systemMessageDisplayed = true; // Set flag to true after displaying the message
    }
  }
}

void resetRXBuffer() {
  b_read = false;
  b_discard = false;
  i = 0;
  j = 0;
  memset(buff, 0, sizeof(buff));
  memset(final_buff, 0, sizeof(final_buff));
}

void clearLine(int line)
{
  lcd.setCursor(0, line);
  lcd.print("                "); // 16 spaces to clear the line
}
