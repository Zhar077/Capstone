#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Definisikan LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Alamat I2C mungkin berbeda, tergantung pada modul Anda

const int buttonPin1 = D7;  // Pin push button
const int triggerPin1 = D6;
bool buttonState1 = false; // Menyimpan status push button
bool systemActive1 = false; // Menyimpan status sistem aktif/non-aktif
bool systemMessageDisplayed = false;
bool readingInProgress = false; // Menyimpan status pembacaan tensi

unsigned long lastDebounceTime = 0; // Waktu terakhir tombol ditekan
unsigned long debounceDelay = 50; // Debounce time (ms)

char buff[64];
bool b_read = false, b_discard = false;
char discard;
int i, j = 0;
char final_buff[64];

int hexSys, hexDias;

void setup() {
  // Inisialisasi Serial Monitor
  Serial.begin(115200);
  // Inisialisasi LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  // Inisialisasi pin push button
  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(triggerPin1, OUTPUT);
}

void clearLine(int line) {
  lcd.setCursor(0, line);
  lcd.print("                "); // 16 spaces to clear the line
}

void resetRXBuffer() {
  b_read = false;
  b_discard = false;
  i = 0;
  j = 0;
  memset(buff, 0, sizeof(buff));
  memset(final_buff, 0, sizeof(final_buff));
}

void loop() {
  // Baca status push button dengan debouncing
  int reading = digitalRead(buttonPin1);
  if (reading == LOW && (millis() - lastDebounceTime) > debounceDelay) {
    lastDebounceTime = millis();
    buttonState1 = !buttonState1;
    systemActive1 = !systemActive1;
    systemMessageDisplayed = false; // Reset flag
    readingInProgress = false; // Reset flag

    clearLine(0); // Clear first line
    clearLine(1); // Clear second line
    if (systemActive1) {
      digitalWrite(triggerPin1, HIGH);
      delay(100);
      digitalWrite(triggerPin1, LOW);
      lcd.setCursor(0, 0);
      lcd.print("Sistem Aktif    ");
      Serial.println("Sistem Aktif");
      Serial.print("Memproses....  ");
      Serial.println("       ");
    } else {
      digitalWrite(triggerPin1, HIGH);
      delay(100);
      digitalWrite(triggerPin1, LOW);
      lcd.setCursor(0, 0);
      lcd.print("Sistem Mati     ");
      Serial.println("Sistem Mati");
      Serial.print("Tekan tombol untuk memulai.. ");
      Serial.println("       ");
      resetRXBuffer(); // Reset RX buffer and related variables
    }
    delay(1000); // Delay untuk menampilkan status sistem
  }

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
