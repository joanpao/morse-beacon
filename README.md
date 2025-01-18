# Morse CW Beacon

Here you can find the sources of a simple CW beacon, for ESP32 board with LoRa and OLED display.
The beacon sends a predefined message as Morse code on a fixed frequency, like 433.575 MHz.
The signal can be received using any FM walkie Talkie (VHF / UHF )

## Hardware Requirements

 * ESP32 board with LoRa SX12xx transceiver and OLED SSD1306 display

Boards are supported:
 * WIFI-LORA32 (original)
 * TTGO-LORA32 (original)
 * Lilygo ESP32 433 / 866 LoRa T3_V1.6.1 (ESP PICO-D4 in Arduino iDE) (Now, tested 144, 430, 433, 466)
 * Lilygo ESP32 866 LoRa T3_V1.6.1 (ESP PICO-D4 in Arduino iDE) (Now, tested 866)
 * Lilygo T-Beam 433 LoRa GPS (Now, tested 144, 430, 433, 466))


## Software Requirements

 * Arduino IDE `>= 1.8.5`
 * Arduino support for ESP32 platform
 * Adafruit GFX Library

## Build

1. Get the sources: `git clone https://github.com/sergev/morse-beacon.git`

2. Select message, random characters or random 5 groups with comment or uncomment the message at end of `morse-beacon.ino` file. 

3. Make sure pin definitions match your ESP32 board (LORA_RST, OLED_SDA, OLED_SCL, LED).

4. Change the frequency to the desired value (FREQ_HZ).

5. Compile and upload the program to the board.

## Credits

*morse-beacon* was created by Serge Vakulenko, KK6ABQ and modified by Juan Pablo Sanchez EA5JSB in 18/1/2025
