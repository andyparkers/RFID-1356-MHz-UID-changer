#include "GyverEncoder.h"
#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>


const int CLK     = 2;         // S2
const int DT      = 3;         // S1
const int SW      = 4;         // KEY
const int RST_PIN = 9;
const int SS_PIN  = 10;


/*
   Display wiring:
   GND - GND, VCC - 5V, SDA - A4, SCL - A5
   Encoder wiring:
   GND - GND, VCC - 5V, S1 - D3, S2 - D2, KEY - D4
   RFID module wiring:
   GND - GND, 3.3V - 3.3V, RST - D9, MISO - D12, MOSI - D11, SCK - D13, SDA - D10

*/


MFRC522 mfrc522(SS_PIN, RST_PIN);
Encoder enc(CLK, DT, SW, TYPE1);
LiquidCrystal_I2C lcd(0x27, 20, 4);

MFRC522::MIFARE_Key key;
MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);

const byte cursor_symbol[8] = {
  0x00, 0x04, 0x08, 0x1F,
  0x08, 0x04, 0x00, 0x00
};

const byte upper_cursor[8] = {
  0x00, 0x04, 0x04, 0x04,
  0x15, 0x0E, 0x04, 0x00
};

const byte tracery[8] = {
  0x0A, 0x15, 0x0A, 0x15,
  0x0A, 0x15, 0x0A, 0x15
};

byte nuid[4] = {0x00, 0x00, 0x00, 0x00};
byte uid_manual[4] = {0x12, 0x34, 0x56, 0x78};


byte cursor_pos = 0;
bool main_menu = 1;
bool cursor_clear = 0;
bool first_enter_submenu = 1;
byte manual_enter_pos = 0;
bool end_of_writing = 0;
bool first_write_manual = 1;
uint64_t timer = 0;
bool first_time_waiting_animation = 1;
byte dot_pos = 6;
bool animation = 1;
bool first_clear_row = 1;

void SetParametersByDefault() {
  end_of_writing = 0;
  first_write_manual = 1;
  main_menu = 1;
  InitialScreen();
  first_enter_submenu = 1;
  manual_enter_pos = 0;
}

void PrintUID(byte* arr, byte x, byte y) {
  for (byte i = 0; i < 4; ++i) {
    lcd.setCursor(i * 2 + x, y);
    String to_print = *(arr + i) > 15 ? String(*(arr + i), HEX)
                      : String("0") + String(*(arr + i), HEX);
    to_print.toUpperCase();
    lcd.print(to_print);
  }
}

void WaitingAnimation() {
  if (first_time_waiting_animation) {
    lcd.clear();
    first_time_waiting_animation = 0;
    lcd.setCursor(7, 1);
    lcd.print("READY");
  }
  if (abs(millis() - timer) >= 300) {
    timer = millis();
    lcd.setCursor(dot_pos, 2);
    lcd.print(" ");
    dot_pos = (dot_pos < 11) ? dot_pos + 1 : 7;
    lcd.setCursor(dot_pos, 2);
    lcd.print(".");
  }
}

void PrintStar(int x, int y, uint64_t anumation_duration) {
  lcd.setCursor(x, y);
  lcd.print("*");
  delay(anumation_duration);
  lcd.setCursor(x, y);
  lcd.print(" ");
}

[[nodiscard]] bool WaitForCard() {
  if (!mfrc522.PICC_IsNewCardPresent()) {
    if (animation) {
      WaitingAnimation();
    }
    return 1;
  }

  if (!mfrc522.PICC_ReadCardSerial()) {
    return 1;
  }
  return 0;
}

void ReadCardUID() {
  enc.tick();
  if (WaitForCard()) {
    return;
  }

  first_time_waiting_animation = 1;
  dot_pos = 6;

  for (int i = 0; i < 4; ++i) {
    nuid[i] = mfrc522.uid.uidByte[i];
  }

  lcd.setCursor(5, 1);
  lcd.print("Card UID:");
  PrintUID(nuid, 6, 2);
  PrintStar(17, 1, 1000);

  animation = 0;
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

void WriteCardUID(byte* arr = nuid) {
  enc.tick();
  if (WaitForCard()) {
    return;
  }

  first_time_waiting_animation = 1;
  dot_pos = 6;
  animation = 0;
  PrintUID(arr, 6, 2);
  if (mfrc522.MIFARE_SetUid(arr, (byte)4, true)) {
    lcd.setCursor(6, 1);
    lcd.print("       ");
    lcd.setCursor(6, 0);
    lcd.print("Written!");
  }
  else {
    lcd.setCursor(6, 0);
    lcd.print("        ");
    lcd.setCursor(6, 1);
    lcd.print("Failed!");
  }
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  PrintStar(17, 1, 1000);
}

void PrintUpperArrow(byte x) {
  if (x != 6) {
    lcd.setCursor(x - 1, 1);
    lcd.print(" ");
  }
  lcd.setCursor(x, 1);
  lcd.write(2);
}

void WriteCardUIDManual() {
  enc.tick();
  if (first_write_manual) {
    first_write_manual = 0;
    lcd.clear();
    PrintUID(uid_manual, 6, 2);
    lcd.setCursor(2, 0);
    lcd.print("Enter UID below:");
  }
  if (!end_of_writing) {
    first_clear_row = 1;
    PrintUpperArrow(manual_enter_pos + 6);
    byte additional_number = (manual_enter_pos % 2 == 0) ? 16 : 1;
    if (enc.isRight()) {
      if ((uid_manual[manual_enter_pos / 2] & (0x0F << (additional_number / 4)))
          >> (additional_number / 4) < 15) {
        uid_manual[manual_enter_pos / 2] += additional_number;
      }
      enc.tick();
      PrintUID(uid_manual, 6, 2);
    }
    else if (enc.isLeft()) {
      if ((uid_manual[manual_enter_pos / 2] & (0x0F << (additional_number / 4)))
          >> (additional_number / 4) > 0) {
        uid_manual[manual_enter_pos / 2] -= additional_number;
      }
      enc.tick();
      PrintUID(uid_manual, 6, 2);
    }
    enc.tick();
    if (enc.isClick()) {
      manual_enter_pos += 1;
      if (manual_enter_pos == 8) {
        end_of_writing = 1;
      }
    }
  }
  else {
    if (first_clear_row) {
      lcd.setCursor(2, 0);
      lcd.print("                ");
      first_clear_row = 0;
    }
    lcd.setCursor(13, 1);
    lcd.print(" ");
    WriteCardUID(uid_manual);
    enc.tick();
    if (enc.isHolded()) {
      SetParametersByDefault();
    }
  }
  if (enc.isHolded()) {
    SetParametersByDefault();
  }
}

void Actions(byte act) {
  enc.tick();
  if (!main_menu) {
    if (act == 0) {
      ReadCardUID();
    }
    else if (act == 1) {
      WriteCardUID();
    }
    else if (act == 2) {
      WriteCardUIDManual();
    }
  }
}

void ProcessInput() {
  enc.tick();
  if (first_enter_submenu) {
    first_enter_submenu = 0;
    first_time_waiting_animation = 1;
    lcd.clear();
  }
}

void InitialScreen() {
  enc.tick();
  lcd.clear();
  PrintCursor(cursor_pos);
  lcd.setCursor(0, 0);
  lcd.print("Read UID");
  lcd.setCursor(0, 1);
  lcd.print("Write UID");
  lcd.setCursor(0, 2);
  lcd.print("Write UID manual");
}

void PrintCursor(byte cursor_position) {
  enc.tick();
  if (main_menu) {
    animation = 1;
    switch (cursor_position) {
      case 0: {
          lcd.setCursor(18, 0);
          lcd.write(1);
          lcd.setCursor(18, 1);
          lcd.print(" ");
          lcd.setCursor(18, 2);
          lcd.print(" ");
          break;
        }
      case 1: {
          lcd.setCursor(18, 1);
          lcd.write(1);
          lcd.setCursor(18, 0);
          lcd.print(" ");
          lcd.setCursor(18, 2);
          lcd.print(" ");
          break;
        }
      case 2: {
          lcd.setCursor(18, 2);
          lcd.write(1);
          lcd.setCursor(18, 0);
          lcd.print(" ");
          lcd.setCursor(18, 1);
          lcd.print(" ");
          break;
        }
    }
  }
}

void ProcessCursor(byte* par) {
  enc.tick();
  if (enc.isClick() && (main_menu == 1 || cursor_pos != 2)) {
    main_menu = !main_menu;
    if (main_menu == 1) {
      InitialScreen();
      first_enter_submenu = 1;
    }
  }
  if (main_menu) {
    cursor_clear = 0;
    if (enc.isRight()) {
      *par = *par < 2 ? *par + 1 : 0;
      PrintCursor(*par);
    }
    else if (enc.isLeft()) {
      *par = *par > 0 ? *par - 1 : 2;
      PrintCursor(*par);
    }
  }
  else {
    if (!cursor_clear) {
      lcd.setCursor(18, 0);
      lcd.write(" ");
      lcd.setCursor(18, 1);
      lcd.print(" ");
      lcd.setCursor(18, 2);
      lcd.print(" ");
      cursor_clear = 1;
    }
    ProcessInput();
  }
}

void setup() {
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  SPI.begin();
  lcd.begin();
  lcd.createChar(1, cursor_symbol);
  lcd.createChar(2, upper_cursor);
  lcd.createChar(3, tracery);
  mfrc522.PCD_Init();
  lcd.backlight();
  InitialScreen();
}

void loop() {
  enc.tick();
  ProcessCursor(&cursor_pos);
  Actions(cursor_pos);
}
