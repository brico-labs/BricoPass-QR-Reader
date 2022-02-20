# BricoPass-QR-Reader
QR reader to validate BricoLabs members at entrance without contact.

Sends QR code to server via MQTT and waits validation. It shows info with screen, GM61 ligths and buzzer

## Hardware
- TTGO T Display (ESP32)
- GM61 QR scanner (grow tech)
- Buzzer

## Connections
- GM61
 - RX to ESP32 pin 27
 - TX to ESP32 pin 26
 - Vcc to ESP32 3V3
 - GND to ESP32 GND
- Buzzer
 - + to ESP32 pin 12
 - - to ESP32 GND

## How To

- Install vscode with Platform.io https://platformio.org/install/ide?install=vscode
- Clone this repo and open the folder with vscode
- Copy `src/credentials.h.dist` to `src/credentials.h` and replace the ssid, users and passwords
- Plug the board in
- Click build in the tool bar https://docs.platformio.org/en/latest/integration/ide/vscode.html#platformio-toolbar
- Finally click upload and open the serial monitor to read the debug logs