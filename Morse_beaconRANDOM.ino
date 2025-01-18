/*
 * Morse CW beacon for LoRa ESP32 board with OLED display.
 *
 * Copyright (c) 2018 Serge Vakulenko
 *
 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL
 * THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
// ----------------------------------------
//  Source modified by  EA5JTT in 20250118
//  - For work with :
//    Lilygo ESP32 433 / 866 LoRa T3_V1.6.1 (ESP PICO-D4 in Arduino iDE)
//    Lilygo T-Beam LoRa GPS
//  - Tested TX in: 144, 430, 446 & 866 MHz
//  - For send continous random 5 characters group
//  - Completed Morse Code Table
//  Next Projects:
//  - Read GPS/GNSS time and location and transmision in Morse Code
//  More projects and ideeas in https://sonotrigger-software.blogspot.com/
//  You can use this program, but you must keep this announcement
// ----------------------------------------
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_SSD1306.h"
#include "Fixed8x16.h"
#include "LoRa.h"

// Board pins ESP32 433 LORA32 V2.1_1.6
// LED pins 
#define LED 25
// LoRa pins.
#define LORA_SCK 5    // IO5  -- SCLK
#define LORA_MISO 19  // IO19 -- MISO
#define LORA_MOSI 27  // IO27 -- MOSI
#define LORA_SS 18    // IO18 -- CS
// #define LORA_RST    12  // IO12 -- RESET for V2.1 (use IO14 for V2.0)
#define LORA_RST 23  // IO12 -- RST RESET for V2.1 (use IO14 for V2.0)
#define LORA_DI0 26  // IO26 -- DIO Interrupt Request
// OLED pins.
// https://gist.github.com/tlrobinson/dd8cf7d0638bdf49f64812a77f7a798c
#define OLED_RST NOT_A_PIN  // IO16 -- RESET
#define OLED_SDA 21         // IO21 -- SDA for TTGO Lora32 board
#define OLED_SCL 22         // IO22 -- SCL

// LoRa frecuency
//
// EU433 (433.05 - 434.79 MHz) (Free transmision)
// ETSI has defined 433 to 434 MHz frequency band for LoRa application.
// It uses 433.175 MHz, 433.375 MHz and 433.575 MHz frequency channels.
//
// EU863 (863 - 870 MHz) (Free transmision)
//
// HAM band (only with license)
// 2m 144 - 146 MHz Beacon in 144.400 - 144.491
// 70 cm 430 - 440 MHz BEacon in 432.400 - 432.490
//
// PMR band (Free transmision)
// PMR 446 432.400 - 432.490
//
#define FREQ_HZ     144400000   // 144.400 MHz
//#define FREQ_HZ 432400000   // 144.400 MHz
//#define FREQ_HZ 866500000  // 866.500 MHz
//
// Max TX Power in Lyligo ESP32 LoRa is 20 dBm = 100 mW
// Tested:  11dBm 12,5 mW, 14 25 mW, 17dBm 50 mW, 20 dBm 100 mW
#define TX_DBM 20
//
// Morse speed, words per minute.
//
#define WPM 12

Adafruit_SSD1306 display(OLED_RST);

// long time definition: dot, dash ans pause 

int dot_msec = 1200 / WPM;
int dash_msec = 3600 / WPM;
int pause_msec = 3600 / WPM;

int space_seen;

// for random characters generation 
const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.,;:/?=";
const int alphabetLength = sizeof(alphabet) - 1;  // -1 para evitar el carácter nulo

void setup() {
  // Set LED pin.
  pinMode(LED, OUTPUT);

  // Initialize serial port.
  Serial.begin(115200);
  while (!Serial)
    ;
  // Write in serial monitor
  Serial.println();
  delay(1000);
  Serial.println("LoRa Morse Beacon v1.0, by Serge Vakulenko KK6ABQ");
  Serial.println("Mod JSB 12102024");
  // Initialize LoRa chip.
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DI0);
  if (!LoRa.begin(FREQ_HZ)) {
    Serial.println("Starting LoRa failed!");
    while (1)
      ;
    // random seed generation
    randomSeed(analogRead(0));  
  }
  LoRa.setSignalBandwidth(41700);
  LoRa.setCodingRate4(5);
  LoRa.setSpreadingFactor(6);
  LoRa.disableCrc();
  LoRa.setTxPower(TX_DBM);
  Serial.println("init ok");

  // Initialize OLED display with the I2C address 0x3C.
  // OLED_SDA, OLED_SCL son las variables ya inicializadas con los pines usados apra OLED, el 21 y 22 en este caso
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C, true, OLED_SDA, OLED_SCL);
  display.clearDisplay();

  // Set font.
  // text size 8x16
  // WHITE color because B/W display
  display.setFont(&Fixed8x16);
  display.setTextColor(WHITE);
  display.setCursor(0, Fixed8x16.yAdvance);

  // Visualize the result.
  display.display();
}

void beep(int msec) {
  // Compute preamble length of required duration.
  // To measure the timing, use plen = msec.
  int plen = (1000L * msec - 19089) / 1536;

  digitalWrite(LED, HIGH);  // LED on

  LoRa.setPreambleLength(plen);  // preamble length
  LoRa.beginPacket(true);        // no header
  LoRa.write(0);                 // data 1 byte
  LoRa.endPacket();

  digitalWrite(LED, LOW);  // LED off
}

void send_morse(char *sym, char *str) {
  display.print(sym);
  display.display();
  Serial.print(sym);

  if (*str++ == '.')
    beep(dot_msec);
  else
    beep(dash_msec);

  while (*str) {
    delay(dot_msec);

    if (*str++ == '.')
      beep(dot_msec);
    else
      beep(dash_msec);
  }
}

void send_char(int c) {
  if (isspace(c) || !isprint(c))
    c = ' ';
  else if (isalpha(c) && islower(c))
    c = toupper(c);

  switch (c) {
    default:
      return;
    case ' ':
      if (!space_seen) {
        display.write(' ');
        display.display();
        Serial.write(c);
        delay(dot_msec + pause_msec);
      }
      space_seen = 1;
      return;
// MORSE CODE
// https://www.itu.int/dms_pubrec/itu-r/rec/m/r-rec-m.1677-1-200910-i!!pdf-e.pdf
    case 'A': send_morse("A", ".-"); break;
    case 'B': send_morse("B", "-..."); break;
    case 'C': send_morse("C", "-.-."); break;
    case 'Ç': send_morse("Ç", "-.-.."); break;
    case 'D': send_morse("D", "-.."); break;
    case 'E': send_morse("E", "."); break;
    case 'F': send_morse("F", "..-."); break;
    case 'G': send_morse("G", "--."); break;
    case 'H': send_morse("H", "...."); break;
    case 'I': send_morse("I", ".."); break;
    case 'J': send_morse("J", ".---"); break;
    case 'K': send_morse("K", "-.-"); break;
    case 'L': send_morse("L", ".-.."); break;
    case 'M': send_morse("M", "--"); break;
    case 'N': send_morse("N", "-."); break;
    case 'Ñ': send_morse("Ñ", "--.--"); break;
    case 'O': send_morse("O", "---"); break;
    case 'P': send_morse("P", ".--."); break;
    case 'Q': send_morse("Q", "--.-"); break;
    case 'R': send_morse("R", ".-."); break;
    case 'S': send_morse("S", "..."); break;
    case 'T': send_morse("T", "-"); break;
    case 'U': send_morse("U", "..-"); break;
    case 'V': send_morse("V", "...-"); break;
    case 'W': send_morse("W", ".--"); break;
    case 'X': send_morse("X", "-..-"); break;
    case 'Y': send_morse("Y", "-.--"); break;
    case 'Z': send_morse("Z", "--.."); break;
    case '0': send_morse("0", "-----"); break;
    case '1': send_morse("1", ".----"); break;
    case '2': send_morse("2", "..---"); break;
    case '3': send_morse("3", "...--"); break;
    case '4': send_morse("4", "....-"); break;
    case '5': send_morse("5", "....."); break;
    case '6': send_morse("6", "-...."); break;
    case '7': send_morse("7", "--..."); break;
    case '8': send_morse("8", "---.."); break;
    case '9': send_morse("9", "----."); break;
    case '.': send_morse(".", ".-.-.-"); break;
    case ',': send_morse(",", "--..--"); break;
    case ';': send_morse(";", "-.-.-."); break;
    case '?': send_morse("?", "..--.."); break;
    case '/': send_morse("/", "-..-."); break;
    case '=': send_morse("=", "-...-"); break;
    case ':': send_morse(":", "---..."); break;
    case '@': send_morse("@", ".--.-."); break;
    case '-': send_morse("-", "-....-"); break;
    case '(': send_morse("(", "-.--."); break;
    case ')': send_morse(")", "-.--.-"); break;
    case '+': send_morse("+", ".-.-."); break;
    case 'x': send_morse("x", "-..-"); break;
  }
  space_seen = 0;
}

void send_message(const char *str) {
  while (*str) {
    send_char(*str++);
    delay(pause_msec);
  }


  delay(dot_msec + pause_msec);
  display.clearDisplay();
  display.setCursor(0, Fixed8x16.yAdvance);
}
// No limit, tested 5 characters x 58 grups = 348 characters

void loop() {
  // A simple message
  // send_message("Hola mundo");
  //
  // A simple 2 string
  //send_message("YETSR AUSHQ BWJSU APLMQ BWNSH ETSFZ WCQJA UWKQI NDHRT XCWJA ");
  //display.clearDisplay();
  //display.setCursor(0, Fixed8x16.yAdvance);
  //send_message("MQKAU 26438 BXHDY ETXCQ WNBHJ EUIKL AMPLI NWXXV SHATD WNQHA");
  //
  // send continuos random charcters
  //int randomIndex = random(0, alphabetLength);
  //char randomLetter[2]; // Array for character and terminal sign
  //randomLetter[0] = alphabet[randomIndex];
  //randomLetter[1] = '\0'; // terminal sign
  //send_message(randomLetter);
  //
  //send continuos random 5 charcters groups, is a estandar for morse learning
  int randomIndex1 = random(0, alphabetLength);
  int randomIndex2 = random(0, alphabetLength);
  int randomIndex3 = random(0, alphabetLength);
  int randomIndex4 = random(0, alphabetLength);
  int randomIndex5 = random(0, alphabetLength);
  char randomLetter[6];  // Array for character and terminal sign
  randomLetter[0] = alphabet[randomIndex1];
  randomLetter[1] = alphabet[randomIndex2];
  randomLetter[2] = alphabet[randomIndex3];
  randomLetter[3] = alphabet[randomIndex4];
  randomLetter[4] = alphabet[randomIndex5];
  randomLetter[5] = '\0';  // terminal sign
  send_message(randomLetter);
  // each 1 segon
  delay(1000);
}
