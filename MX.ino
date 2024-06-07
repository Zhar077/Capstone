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

const int buttonPin = D6; // Pin push button
int button = 1;
int rep = 0;
bool systemActive = false; // Menyimpan status sistem aktif/non-aktif
unsigned long lastDebounceTime = 0; // Waktu terakhir tombol ditekan
unsigned long debounceDelay = 50; // Debounce time (ms)

bool fingerDetected = false;
bool systemMessageDisplayed = false;



void setup()
{
  pinMode(buttonPin, INPUT_PULLUP); // Konfigurasi pin button dengan pull-up
  Serial.begin(115200);

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

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // You can also specify server:
  // Blynk.begin(BLYNK_AUTH_TOKEN, "blynk-cloud.com", 80);
  // Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, IPAddress(192,168,1,100), 8080);
}

int32_t SPO2;
int8_t SPO2Valid;
int32_t heartRate;
int8_t heartRateValid;

void loop()
{
  //UNTUK MERUBAH STATUS systemActive
  rep = rep + 1;
  if (rep==8){
    return;
  }
  int reading = digitalRead(buttonPin);
  delay(1000);
  if (reading == LOW && (millis() - lastDebounceTime) > debounceDelay) {
    lastDebounceTime = millis();
    button = button +1;

    if(button%2==0){
    systemActive = true;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sistem Aktif    ");
    delay(1000); // Delay untuk menampilkan status sistem
    }

    else {
      systemActive = false;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Sistem Mati     ");
      delay(1000);
    }
  }


  if (!systemActive) {
  lcd.setCursor(0, 0);     
  lcd.print("Tekan Tombol    ");
  lcd.setCursor(0, 1);
  lcd.print("untuk Aktif     ");
  }

  if (systemActive) {
    lcd.clear();
    lcd.setCursor(0, 0);     
    lcd.print("reading....     ");
  
    long irValue = particleSensor.getIR();
    Serial.print("IR=");
    Serial.print(irValue);

    if (irValue > 45000){
      particleSensor.heartrateAndOxygenSaturation(&SPO2, &SPO2Valid, &heartRate, &heartRateValid);
      
      if (heartRateValid == 1) {
        Serial.print(F("heartRate="));
        Serial.print(heartRate, DEC);
        Serial.print(F(", heartRateValid="));
        Serial.println(heartRateValid, DEC);
        
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("HR:");
        lcd.print(heartRate);
        lcd.print(" bpm        ");
        Blynk.virtualWrite(V7, heartRate);
        delay(5000);
      }
    }
    else {
      Serial.println("  No finger?");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("No Finger       ");
    }
  Blynk.run();
  }
}
